#pragma once

#include <cstring>
#include <unordered_map>
#include <memory>
#include <mysql.h>

#include <detail/error.hpp>
#include <detail/convert.hpp>

namespace mysql_detail {
/*
* 有些类型可能会有错，有错的时候在宏里面改, 不支持的类型或者错误类型都会导致异常.
* MYSQL_TYPE_NULL 不知道对应什么类型.
* MYSQL_TYPE_ENUM 枚举类型感觉没什么用的字段, 不支持, 如果需要可以当成字符串.
* MYSQL_TYPE_GEOMETRY 不支持.
*/
//check整数
#define CHECKLONG(t)		(t == MYSQL_TYPE_LONG || t == MYSQL_TYPE_TINY || t == MYSQL_TYPE_SHORT || t == MYSQL_TYPE_INT24)
//check长整数
#define CHECKLONGLONG(t)	(t == MYSQL_TYPE_LONGLONG)
//check浮点数
#define CHECKFLOAT(t)		(t == MYSQL_TYPE_FLOAT)
//check浮点数
#define CHECKDOUBLE(t)		(t == MYSQL_TYPE_DOUBLE || t == MYSQL_TYPE_DECIMAL || t == MYSQL_TYPE_NEWDECIMAL)
//check字符串
#define CHECKSTRING(t)		(t == MYSQL_TYPE_STRING || t == MYSQL_TYPE_VARCHAR || t == MYSQL_TYPE_TINY_BLOB || t == MYSQL_TYPE_VAR_STRING || \
							 t == MYSQL_TYPE_JSON || t == MYSQL_TYPE_BIT || t == MYSQL_TYPE_SET || t == MYSQL_TYPE_MEDIUM_BLOB || \
							 t == MYSQL_TYPE_LONG_BLOB || t == MYSQL_TYPE_BLOB)
//check MYSQL_TIME
#define CHECKMYSQL_TIME(t)	(t == MYSQL_TYPE_TIMESTAMP || t == MYSQL_TYPE_DATE || t == MYSQL_TYPE_TIME || t == MYSQL_TYPE_DATETIME || \
							 t == MYSQL_TYPE_NEWDATE || t == MYSQL_TYPE_TIMESTAMP2 || t == MYSQL_TYPE_DATETIME2 || t == MYSQL_TYPE_TIME2 || \
							 t == MYSQL_TYPE_YEAR)

	template <int t>
	struct int_type : std::integral_constant<bool, CHECKLONG(t)> {};

	template<int t>
	struct long_type : std::integral_constant<bool, CHECKLONG(t)> {};

	template<int t>
	struct longlong_type : std::integral_constant<bool, CHECKLONGLONG(t)> {};

	template<int t>
	struct float_type : std::integral_constant<bool, CHECKFLOAT(t)> {};

	template<int t>
	struct double_type : std::integral_constant<bool, CHECKDOUBLE(t)> {};

	template<int t>
	struct string_type : std::integral_constant<bool, CHECKSTRING(t)> {};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct base_val {
		constexpr static int csize = 8;//数据精度
		base_val(enum_field_types ftype = MYSQL_TYPE_NULL, unsigned long strlen = 0) : ftype_(ftype) {
			std::memset(this->data_.cval_, 0, csize);
			this->strdata_.clear();
			strdata_.resize(strlen);
		}
		//int
		template<typename T, std::enable_if_t<std::is_same<int, std::decay_t<T>>::value, int> = 0>
		int get() const {
			if (!CHECKLONG(ftype_)) {
				throw_mysqlstmt_error(err_code::datatype_error, __FILE__, __LINE__, "data type error. current type:", ftype_);
			}
			return data_.ival_;
		}
		template<typename T, std::enable_if_t<std::is_same<int, std::decay_t<T>>::value, int> = 0>
		void set(T& t) {
			data_.ival_ = t;
		}
		//long
		template<typename T, std::enable_if_t<std::is_same<long, std::decay_t<T>>::value, int> = 0>
		long get() const {
			if (!CHECKLONG(ftype_)) {
				throw_mysqlstmt_error(err_code::datatype_error, __FILE__, __LINE__, "data type error. current type:", ftype_);
			}
			return data_.lval_;
		}
		template<typename T, std::enable_if_t<std::is_same<long, std::decay_t<T>>::value, int> = 0>
		void set(T& t) {
			data_.lval_ = t;
		}
		//long long
		template<typename T, std::enable_if_t<std::is_same<long long, std::decay_t<T>>::value, int> = 0>
		long long get() const {
			if (!CHECKLONG(ftype_) && !CHECKLONGLONG(ftype_)) {
				throw_mysqlstmt_error(err_code::datatype_error, __FILE__, __LINE__, "data type error. current type:", ftype_);
			}
			return data_.llval_;
		}
		template<typename T, std::enable_if_t<std::is_same<long long, std::decay_t<T>>::value, int> = 0>
		void set(T& t) {
			data_.llval_ = t;
		}
		//float
		template<typename T, std::enable_if_t<std::is_same<float, std::decay_t<T>>::value, int> = 0>
		float get() const {
			if (!CHECKFLOAT(ftype_)) {
				throw_mysqlstmt_error(err_code::datatype_error, __FILE__, __LINE__, "data type error. current type:", ftype_);
			}
			return data_.fval_;
		}
		template<typename T, std::enable_if_t<std::is_same<float, std::decay_t<T>>::value, int> = 0>
		void set(T& t) {
			data_.fval_ = t;
		}
		//double
		template<typename T, std::enable_if_t<std::is_same<double, std::decay_t<T>>::value, int> = 0>
		double get() const {
			if (!CHECKDOUBLE(ftype_)) {
				throw_mysqlstmt_error(err_code::datatype_error, __FILE__, __LINE__, "data type error. current type:", ftype_);
			}
			return data_.dval_;
		}
		template<typename T, std::enable_if_t<std::is_same<double, std::decay_t<T>>::value, int> = 0>
		void set(T& t) {
			data_.dval_ = t;
		}
		//string
		template<typename T, std::enable_if_t<mysql_detail::is_string_constructible<T>::value, int> = 0>
		std::string get() const {
			return this->tostring();
		}
		template<typename T, std::enable_if_t<mysql_detail::is_string_constructible<T>::value, int> = 0>
		void set(T& t) {
			this->strdata_ = std::forward<std::string>(t);
		}
		//MYSQL_TIME
		template<typename T, std::enable_if_t<std::is_same<MYSQL_TIME, std::decay_t<T>>::value, int> = 0>
		MYSQL_TIME get() const {
			if (!CHECKMYSQL_TIME(ftype_)) {
				throw_mysqlstmt_error(err_code::datatype_error, __FILE__, __LINE__, "data type error. current type:", ftype_);
			}
			return ts_;
		}
		template<typename T, std::enable_if_t<std::is_same<MYSQL_TIME, std::decay_t<T>>::value, int> = 0>
		void set(T& t) {
			ts_ = t;
		}

		//字符串
		std::string& getbuff() {
			return this->strdata_;
		}
		//其他
		void getbuff(char** buff, int& len) {
			if (CHECKMYSQL_TIME(ftype_)) {
				*buff = (char *)&ts_;
				len = sizeof ts_;
			}
			else {
				*buff = data_.cval_;
				len = csize;
			}
		}

		std::string tostring() const {
			if (CHECKDOUBLE(ftype_)) {
				return std::to_string(data_.dval_);
			}
			else if (CHECKFLOAT(ftype_)) {
				return std::to_string(data_.fval_);
			}
			else if (CHECKSTRING(ftype_)) {
				return this->strdata_;
			}
			else if (CHECKMYSQL_TIME(ftype_)) {
				std::stringstream strtime;
				strtime << ts_.year << "-" << ts_.month << "-" << ts_.day << " " <<
					ts_.hour << ":" << ts_.minute << ":" << ts_.second;
				return strtime.str();
			}
			return std::to_string(data_.llval_);
		}
		
		template<typename T>
		T conv_data() const {
			if (CHECKDOUBLE(ftype_)) {
				return conv_get<T>(data_.dval_);
			}
			else if (CHECKFLOAT(ftype_)) {
				return conv_get<T>(data_.fval_);
			}
			else if (CHECKSTRING(ftype_)) {
				return conv_get<T>(this->strdata_);
			}
			else if (CHECKMYSQL_TIME(ftype_)) {
				return T();
			}
			return conv_get<T>(data_.llval_);
		}

	protected:
		union ftype {
			int ival_;
			float fval_;
			double dval_;
			long lval_;
			long long llval_;
			char cval_[csize];
		} data_;
		std::string strdata_;
		MYSQL_TIME    ts_;

		enum_field_types  ftype_;
	};

	template <typename T, typename...>
	struct check_func_back_ref : std::false_type {};
	template <typename T>
	struct check_func_back_ref<T, base_val> : std::is_reference<typename std::invoke_result<decltype(&base_val::get<T>)(base_val)>::type> {};

	struct field_val : public base_val, public type_error {
		using super = type_error;
		explicit field_val(enum_field_types ftype, unsigned long strlen = 0) : base_val(ftype, strlen), super() {
		}
		explicit field_val(err_code ec) : super(ec) {
		}
		explicit field_val(error_code&& ec) : super(std::forward<error_code>(ec)) {
		}

		operator std::string() const {
			return (*this).template get<std::string>();
		}
		
		template <typename T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<T>>, mysql_detail::neg<mysql_detail::is_error<T>>, mysql_detail::neg<check_func_back_ref<T>>> = mysql_detail::enabler>
		operator T() const {
			return this->template get<T>();
		}
		
		template <typename T, mysql_detail::enable<mysql_detail::neg<mysql_detail::is_string_constructible<T>>, mysql_detail::neg<mysql_detail::is_error<T>>, check_func_back_ref<T>> = mysql_detail::enabler>
		operator T&() const {
			return this->template get<T&>();
		}
		template<class TV>
		auto& operator=(TV t) {
			//this->set(t);//只允许stmt赋值
			return *this;
		}
	};
	static field_val error_field_val(err_code::no_data);
	typedef std::shared_ptr<field_val> FIELDVALPTR;

	//一行数据
	struct rows_ret {
		rows_ret() = default;
		~rows_ret() = default;

		void reset() {
			this->all_data_.clear();
		}

		bool add_field(std::string& name, FIELDVALPTR& field) {
			bool inserted = this->all_data_.try_emplace(name, field).second;
			return (inserted ? 1 : 0);
		}
		auto& operator[](std::string&& name) const {
			auto item = all_data_.find(name);
			if (item == all_data_.end()) {
				return error_field_val;
			}
			return *item->second;
		}
	private:
		std::unordered_map<std::string, FIELDVALPTR> all_data_;
	};
	typedef std::shared_ptr<rows_ret> ROWSRETPTR;
}
