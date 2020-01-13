#pragma once

#include <sstream>

#define  IS_THROW_EXCEPT
#ifdef IS_THROW_EXCEPT
#define STMT_TRY try
#define STMT_CATCH(x) catch (x)
#else
#define STMT_TRY
#define STMT_CATCH(x)
#endif
namespace mysql_detail {
	using error_code = std::error_code;
	using system_error = std::system_error;
	template <typename Exception>
	inline void throw_exception(const Exception& e) {
		throw e;
	}
	enum err_code {
		default_error = 1,
		data_error, 
		no_data, 
		datatype_error, 

		binddata_error = 100, 
		stmtinit_error, 
		stmtprepare_error, 
		invalidparam_error, 
		metadata_error, 
		getcolumns_error,
		execute_error,
		bind_result_error,
		store_result_error,
		getresult_error, //获取结果错误
	};

	class type_error {
	public:
		explicit type_error() = default;
		explicit type_error(error_code&& ec) : ec_(std::forward<error_code>(ec)) {
		}
		explicit type_error(err_code ec) {
			this->set_error(ec);
		}
		operator error_code() const {
			return ec_;
		}
		void set_error(const error_code& ec) {
			//ec_.clear();
			ec_ = ec;
		}
		inline void set_error(err_code ec) {
			ec_.assign(ec, std::generic_category());
		}
		template<typename T>
		inline void set_error(int ec, const T& ecat) {
			ec_.assign(ec, ecat);
		}
		inline auto error_val() {
			return ec_.value();
		}
		inline auto error_msg() {
			return ec_.message();
		}
		bool valid() {
			if (this->ec_) {
				return false;
			}
			return true;
		}
	protected:
		error_code ec_;
	};

	class mysqlstmt_error : public std::runtime_error {
	public:
		explicit mysqlstmt_error(int code, const char* message) : std::runtime_error(message), errorcode_(code) {}
		explicit mysqlstmt_error(int code, const std::string& message)
			: std::runtime_error(message), errorcode_(code) {}

		mysqlstmt_error(const mysqlstmt_error&) = default;
		mysqlstmt_error& operator=(const mysqlstmt_error&) = default;
		mysqlstmt_error(mysqlstmt_error&&) = default;
		mysqlstmt_error& operator=(mysqlstmt_error&&) = default;
		~mysqlstmt_error() noexcept override {}

		int ecode() const { return errorcode_; }
	private:
		int errorcode_ = 0;
	};

	// 抛出异常
	template<class ...Args>
	inline void throw_mysqlstmt_error(int code, Args&&... args) {
		std::stringstream stream;
		((stream << std::forward<Args>(args) << ": "), ...);
		std::cerr << stream.str() << std::endl;
#ifdef IS_THROW_EXCEPT
		throw_exception(mysqlstmt_error(code, stream.str()));
#endif // IS_THROW_EXCEPT
	}
}
