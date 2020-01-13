#pragma once

#include <mysql.h>
#include <string>
#include <cstring>
#include <functional>
#include <iostream>
#include <tuple>

#include <detail/traits.hpp>
#include <detail/field.hpp>

namespace mysql_detail {
	struct mysql_bind_proxy {
		MYSQL_BIND* binds_ = nullptr;
		int count_ = 0;

		mysql_bind_proxy() {}
		mysql_bind_proxy(const mysql_bind_proxy&) = delete;
		mysql_bind_proxy& operator=(const mysql_bind_proxy&) = delete;
		~mysql_bind_proxy() {
			if (binds_) delete[] binds_;
			count_ = 0;
		}
		mysql_bind_proxy(mysql_bind_proxy& x) : binds_(x.binds_), count_(x.count_) {
			x.binds_ = nullptr;
		}
		mysql_bind_proxy& operator=(mysql_bind_proxy& x) {
			count_ = x.count_;
			binds_ = x.binds_;
			x.binds_ = nullptr;
			return *this;
		}
		inline bool init(int count) {
			count_ = count;
			if (count <= 0) {
				return false;
			}
			binds_ = (new MYSQL_BIND[count_]);
			if (!binds_) {
				return false;
			}
			std::memset(binds_, 0, count_ * sizeof(MYSQL_BIND));
			return true;
		}
		// 整数
		template<typename T, std::enable_if_t<std::is_same<int, std::decay_t<T>>::value, bool> = true>
		inline void  bindparam(int idx, T& t) {
			bind_param(idx, MYSQL_TYPE_LONG, (char*)&t, 0, 0, 0);
		}
		// 长整数
		template<typename T, std::enable_if_t<std::is_same<long long, std::decay_t<T>>::value, bool> = true>
		inline void  bindparam(int idx, T& t) {
			bind_param(idx, MYSQL_TYPE_LONGLONG, (char*)&t, 0, 0, 0);
		}
		// 浮点
		template<typename T, std::enable_if_t<std::is_same<float, std::decay_t<T>>::value, bool> = true>
		inline void  bindparam(int idx, T& t) {
			bind_param(idx, MYSQL_TYPE_FLOAT, (char*)&t, 0, 0, 0);
		}
		// 浮点
		template<typename T, std::enable_if_t<std::is_same<double, std::decay_t<T>>::value, bool> = true>
		inline void  bindparam(int idx, T& t) {
			bind_param(idx, MYSQL_TYPE_DOUBLE, (char*)&t, 0, 0, 0);
		}
		//字符串
		template<typename T, std::enable_if_t<mysql_detail::is_string_constructible<std::decay_t<T>>::value, bool> = true>
		inline void  bindparam(int idx, T& t) {
			std::string strt(t);
			bind_param(idx, MYSQL_TYPE_STRING, (char*)strt.c_str(), strt.length(), 0, &(binds_[idx].buffer_length));
		}
		template<class Tuple, std::size_t... Is>
		inline void bind_all_t(const Tuple& t, std::index_sequence<Is...>) {
			(this->bindparam(Is, std::get<Is>(t)), ...);
		}
		template<class ...Args>
		inline bool bind_all(Args&&... args) {
			if (this->count_ <= 0) {
				return true;
			}
			int allcount = sizeof...(args);
			if (allcount < this->count_) {//绑定参数个数不匹配
				throw_mysqlstmt_error(err_code::binddata_error, __FILE__, __LINE__, "bind_all error", allcount, count_);
				return false;
			}
			std::tuple<Args...> tp(std::forward_as_tuple(std::forward<Args>(args)...));
			this->bind_all_t(tp, std::index_sequence_for<Args...>{});
			return true;
		}
	//private:
		void bind_param(int i, enum_field_types buffer_type, void* buffer, int buffer_length,
			my_bool* is_null, long unsigned int* length) {
			if (i >= count_) {
				std::cerr << "stmt_proxy::bind_param error! index:" << i << ", count:" << count_ << std::endl;
				return;
			}
			MYSQL_BIND& b = binds_[i];
			b.buffer_type = buffer_type;
			b.buffer = (char*)buffer;
			b.buffer_length = buffer_length;
			b.is_null = is_null;
			b.length = length;
		}
	};

	typedef std::function<void(int)> foreach_cb;
	typedef std::function<void(int, const ROWSRETPTR&)> get_rows_cb;
	//查询
	class stmt_query_proxy {
	public:
		stmt_query_proxy() : stmt_(nullptr), prepare_meta_result_(nullptr), bindproxy_(), bindparamproxy_() {}
		~stmt_query_proxy() {
			if (stmt_) mysql_stmt_close(stmt_);
			if(prepare_meta_result_) mysql_free_result(prepare_meta_result_);
		}
		stmt_query_proxy(const stmt_query_proxy&) = delete;
		stmt_query_proxy& operator=(const stmt_query_proxy&) = delete;

		stmt_query_proxy(stmt_query_proxy&& x) : stmt_(x.stmt_)
			, prepare_meta_result_(x.prepare_meta_result_)
			, bindproxy_(x.bindproxy_)
			, bindparamproxy_(x.bindparamproxy_)
			, initflag_(x.initflag_) {
			x.stmt_ = nullptr;
			x.prepare_meta_result_ = nullptr;
			x.bindproxy_.binds_ = nullptr;
			x.bindparamproxy_.binds_ = nullptr;
		}

		stmt_query_proxy& operator=(stmt_query_proxy&& x) {
			stmt_ = x.stmt_;
			bindproxy_ = x.bindproxy_;
			bindparamproxy_ = x.bindparamproxy_;
			prepare_meta_result_ = x.prepare_meta_result_;
			initflag_ = x.initflag_;
			x.stmt_ = nullptr;
			x.prepare_meta_result_ = nullptr;
			x.bindproxy_.binds_ = nullptr;
			x.bindparamproxy_.binds_ = nullptr;
			return *this;
		}

		bool init(MYSQL* mysql, const std::string &sql) {
			STMT_TRY{
				this->stmt_ = mysql_stmt_init(mysql);
				if (!stmt_) {
					throw_mysqlstmt_error(err_code::stmtinit_error, __FILE__, __LINE__, "mysql_stmt_init(), out of memory");
					return false;
				}
				if (mysql_stmt_prepare(stmt_, sql.c_str(), sql.size())) {
					throw_mysqlstmt_error(err_code::stmtprepare_error, __FILE__, __LINE__, "mysql_stmt_prepare(), SELECT failed. error msg:",
						mysql_stmt_error(stmt_));
					return false;
				}
				auto param_count = mysql_stmt_param_count(stmt_);
				std::cout << "total parameters in SELECT: " << param_count << std::endl;
				if (param_count > 0 && !bindparamproxy_.init(param_count)) {
					throw_mysqlstmt_error(err_code::invalidparam_error, __FILE__, __LINE__, "invalid parameter count returned by MySQL");
					return false;
				}
				prepare_meta_result_ = mysql_stmt_result_metadata(stmt_);
				if (!prepare_meta_result_) {
					throw_mysqlstmt_error(err_code::metadata_error, __FILE__, __LINE__, "mysql_stmt_result_metadata(), returned no meta information");
					return false;
				}
				auto column_count = mysql_num_fields(prepare_meta_result_);
				std::cout << "total columns in SELECT statement: " << column_count << std::endl;
				if (column_count > 0 && !bindproxy_.init(column_count)) {
					throw_mysqlstmt_error(err_code::getcolumns_error, __FILE__, __LINE__, "get total columns bind data fail.");
					return false;
				}
				initflag_ = true;
				return true;
			}
			STMT_CATCH(const mysql_detail::mysqlstmt_error& e) {
				return false;
			}
		}

		template<class ...Args>
		inline bool query(Args&&... args) {
			STMT_TRY{
				if (!this->bind_param_all(std::forward<Args>(args)...)) { //绑定需要参数
					throw_mysqlstmt_error(err_code::binddata_error, __FILE__, __LINE__, "bind param data fail.");
					return false;
				}
				if (bindparamproxy_.binds_ && mysql_stmt_bind_param(stmt_, bindparamproxy_.binds_)) {
					throw_mysqlstmt_error(err_code::binddata_error, __FILE__, __LINE__, "bind param data fail.");
					return false;
				}
				if (mysql_stmt_execute(stmt_)) {
					throw_mysqlstmt_error(err_code::execute_error, __FILE__, __LINE__, "query faild. errormsg", mysql_stmt_error(stmt_));
					return false;
				}
				return true;
			}
			STMT_CATCH(const mysql_detail::mysqlstmt_error& e) {
				return false;
			}
		}

		//遍历，获取所有结果
		template<class ...Args>
		bool for_each(foreach_cb cbfunc, Args&&... args) {
			STMT_TRY{
				if (!this->bind_all(std::forward<Args>(args)...)) { //绑定需要参数
					throw_mysqlstmt_error(err_code::binddata_error, __FILE__, __LINE__, "for_each bind ret data fail.");
					return false;
				}
				if (mysql_stmt_bind_result(stmt_, bindproxy_.binds_)) {
					throw_mysqlstmt_error(err_code::bind_result_error, __FILE__, __LINE__, "mysql_stmt_bind_result() failed. errormsg",
						mysql_stmt_error(stmt_));
					return false;
				}
				if (mysql_stmt_store_result(stmt_)) {
					throw_mysqlstmt_error(err_code::store_result_error, __FILE__, __LINE__, "mysql_stmt_store_result() failed. errormsg",
						mysql_stmt_error(stmt_));
					return false;
				}
				uint64_t coutrow = 0;
				while (!mysql_stmt_fetch(stmt_)) {
					cbfunc(coutrow);
					coutrow++;
				}
				//check 一般绑定返回类型错误会出现
				auto allcount = mysql_stmt_num_rows(stmt_);
				if (coutrow != allcount) {
					throw_mysqlstmt_error(err_code::getresult_error, __FILE__, __LINE__, "for_each get result fail.");
					return false;
				}
				return true;
			}
			STMT_CATCH(const mysql_detail::mysqlstmt_error& e) {
				return false;
			}
		}

		bool get_result(get_rows_cb cbfunc) {
			STMT_TRY{
				if (!prepare_meta_result_) {
					throw_mysqlstmt_error(err_code::metadata_error, __FILE__, __LINE__, "prepare_meta_result_ is null");
					return false;
				}
				ROWSRETPTR rowret = std::make_shared<rows_ret>();
				unsigned long fieldCount = mysql_num_fields(prepare_meta_result_);
				MYSQL_FIELD*  fields = mysql_fetch_fields(prepare_meta_result_);
				for (uint32_t i = 0; i < fieldCount; i++) {
					auto ftype = fields[i].type;
					std::string fname = fields[i].name;
					unsigned long flen = fields[i].length;
					FIELDVALPTR fvalptr = nullptr;
					if (CHECKSTRING(ftype)) {
						fvalptr = std::make_shared<field_val>(ftype, flen);
						auto& strbuff = fvalptr->getbuff();
						bindproxy_.bindparam(i, strbuff);
					}
					else {
						fvalptr = std::make_shared<field_val>(ftype);
						char* buff = nullptr;
						int len = 0;
						fvalptr->getbuff(&buff, len);
						bindproxy_.bind_param(i, ftype, buff, 0, 0, 0);
					}
					if (fvalptr) {
						rowret->add_field(fname, fvalptr);
					}
				}
				if (mysql_stmt_bind_result(stmt_, bindproxy_.binds_)) {
					throw_mysqlstmt_error(err_code::bind_result_error, __FILE__, __LINE__, "get_result bind result fail",
						mysql_stmt_error(stmt_));
					return false;
				}
				if (mysql_stmt_store_result(stmt_)) {
					throw_mysqlstmt_error(err_code::store_result_error, __FILE__, __LINE__, "mysql_stmt_store_result() failed. errormsg",
						mysql_stmt_error(stmt_));
					return false;
				}
				uint64_t coutrow = 0;
				while (!mysql_stmt_fetch(stmt_)) {
					cbfunc(coutrow, rowret);
					coutrow++;
				}
				//check 一般绑定返回类型错误会出现
				auto allcount = mysql_stmt_num_rows(stmt_);
				if (coutrow != allcount) {
					throw_mysqlstmt_error(err_code::getresult_error, __FILE__, __LINE__, "get_result failed. errormsg:",
						mysql_stmt_error(stmt_));
					return false;
				}
				return true;
			}
			STMT_CATCH(const mysql_detail::mysqlstmt_error& e) {
				return false;
			}
		}

		operator bool() {
			return initflag_;
		}

	protected:
		template<class ...Args>
		inline bool bind_all(Args&&... args) {
			return bindproxy_.bind_all(std::forward<Args>(args)...);
		}
		template<class ...Args>
		inline bool bind_param_all(Args&&... args) {
			return bindparamproxy_.bind_all(std::forward<Args>(args)...);
		}
	private:
		MYSQL_STMT* stmt_;
		MYSQL_RES     *prepare_meta_result_;
		mysql_bind_proxy bindproxy_; 
		mysql_bind_proxy bindparamproxy_;

		bool initflag_ = false;
	};

	class stmt_execute_proxy {
	public:
		stmt_execute_proxy() : stmt_(nullptr), bindproxy_(), affected_rows_(0) {}
		~stmt_execute_proxy() {
			if (stmt_) mysql_stmt_close(stmt_);
		}
		stmt_execute_proxy(const stmt_execute_proxy&) = delete;
		stmt_execute_proxy& operator=(const stmt_execute_proxy&) = delete;

		stmt_execute_proxy(stmt_execute_proxy&& x) : stmt_(x.stmt_)
			, bindproxy_(x.bindproxy_)
			, affected_rows_(x.affected_rows_) 
			, initflag_(x.initflag_) {
			x.stmt_ = nullptr;
			x.bindproxy_.binds_ = nullptr;
		}

		stmt_execute_proxy& operator=(stmt_execute_proxy&& x) {
			stmt_ = x.stmt_;
			bindproxy_ = x.bindproxy_;
			affected_rows_ = x.affected_rows_;
			initflag_ = x.initflag_;
			x.stmt_ = nullptr;
			x.bindproxy_.binds_ = nullptr;
			return *this;
		}

		bool init(MYSQL* mysql, const std::string &sql) {
			STMT_TRY{
				this->stmt_ = mysql_stmt_init(mysql);
				if (!stmt_) {
					throw_mysqlstmt_error(err_code::stmtinit_error, __FILE__, __LINE__, "mysql_stmt_init(), out of memory");
					return false;
				}
				if (mysql_stmt_prepare(stmt_, sql.c_str(), sql.size())) {
					throw_mysqlstmt_error(err_code::stmtprepare_error, __FILE__, __LINE__, "mysql_stmt_prepare(), INSERT failed. error msg:",
						mysql_stmt_error(stmt_));
					return false;
				}
				auto param_count = mysql_stmt_param_count(stmt_);
				std::cout << "total parameters in INSERT: " << param_count << std::endl;
				if (param_count > 0 && !bindproxy_.init(param_count)) {
					throw_mysqlstmt_error(err_code::invalidparam_error, __FILE__, __LINE__, "invalid parameter count returned by MySQL");
					return false;
				}
				initflag_ = true;
				return true;
			}
			STMT_CATCH(const mysql_detail::mysqlstmt_error& e) {
				return false;
			}
		}

		template<class ...Args>
		inline int execute(Args&&... args) {
			STMT_TRY{
				if (!this->bind_all(std::forward<Args>(args)...)) { //绑定需要参数
					throw_mysqlstmt_error(err_code::binddata_error, __FILE__, __LINE__, "execute bind param data fail. errormsg",
						mysql_stmt_error(stmt_));
					return 0;
				}
				if (bindproxy_.binds_ && mysql_stmt_bind_param(stmt_, bindproxy_.binds_)) {
					throw_mysqlstmt_error(err_code::binddata_error, __FILE__, __LINE__, "mysql_stmt_bind_param() failed. errormsg",
						mysql_stmt_error(stmt_));
					return -1;
				}
				if (mysql_stmt_execute(stmt_)) {
					throw_mysqlstmt_error(err_code::execute_error, __FILE__, __LINE__, "execute faild. errormsg", mysql_stmt_error(stmt_));
					return -2;
				}
				affected_rows_ = mysql_stmt_affected_rows(stmt_);
				std::cout << "total affected rows: " << this->affected_rows_ << std::endl;
				return affected_rows_;
			}
			STMT_CATCH(const mysql_detail::mysqlstmt_error& e) {
				return -3;
			}
		}
		
		operator bool() {
			return initflag_;
		}
	protected:
		template<class ...Args>
		inline bool bind_all(Args&&... args) {
			return bindproxy_.bind_all(std::forward<Args>(args)...);
		}
	private:
		MYSQL_STMT* stmt_;
		mysql_bind_proxy bindproxy_;
		my_ulonglong  affected_rows_;

		bool initflag_ = false;
	};
}
