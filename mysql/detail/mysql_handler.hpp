#pragma once

#include <mysql.h>
#include <string>
#include <thread>
#include <mutex>

#include <detail/mysql_stmt.hpp>

namespace mysql_detail {
	class mysql_handler{
	public:
		mysql_handler(std::string host,
			unsigned int port, std::string user, std::string passwd,
			std::string db, std::string character)
			: _host(host),
			  _port(port),
			  _user(user),
			  _passwd(passwd),
			  _db(db),
			  _character(character) {
			static std::once_flag flag;
			std::call_once(flag, []() { mysql_library_init(0, nullptr, nullptr); });

			this->_mysql = mysql_init(NULL);
			mysql_thread_init();

			my_bool reconnect = 1;
			mysql_options(this->_mysql, MYSQL_OPT_RECONNECT, &reconnect);

			/*unsigned int timeout = 10;
			mysql_options(_mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
			mysql_options(_mysql, MYSQL_OPT_READ_TIMEOUT, &timeout);
			mysql_options(_mysql, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
			mysql_options(_mysql, MYSQL_SET_CHARSET_NAME, "utf8");*/
		}
		~mysql_handler() {
			mysql_close(this->_mysql);
			this->_mysql = nullptr;
			mysql_thread_end();
		}

		inline bool connect() {
			MYSQL* ptmp = mysql_real_connect(this->_mysql, this->_host.c_str(),
				this->_user.c_str(), this->_passwd.c_str(),
				this->_db.c_str(), this->_port, NULL, 0);
			bool res = (ptmp != nullptr);
			if (res) {
				mysql_set_character_set(this->_mysql, _character.c_str());
			}
			return res;
		}

		inline void disconnect() {
			mysql_close(this->_mysql);
		}

		inline std::string escape_string(const std::string &data) {
			std::string escaped_str;
			escaped_str.resize(data.size() * 2 + 1);
			uint64_t len = mysql_real_escape_string(
				this->_mysql, &*escaped_str.begin(), data.data(), data.size());
			escaped_str.resize(len);
			return escaped_str;
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////
		stmt_query_proxy getstmtquery(const std::string &sql) {
			stmt_query_proxy stmt;
			stmt.init(this->_mysql, sql);
			return stmt;
		}
		stmt_execute_proxy getstmtexecute(const std::string &sql) {
			stmt_execute_proxy stmt;
			stmt.init(this->_mysql, sql);
			return stmt;
		}
		const char* geterror() {
			return  mysql_error(_mysql);
		}

		template<class ...Args>
		inline bool query(const std::string &sql, Args&&... args) {
			auto stmt = this->getstmtquery(sql);
			if (!stmt) {
				return false;
			}
			if (!stmt.bind_param_all(std::forward<Args>(args)...)) {
				return false;
			}
			if (!stmt.query()) {
				return false;
			}
			return true;
		}
	private:
		MYSQL* _mysql;
		std::string _host;
		unsigned int _port;
		std::string _user;
		std::string _passwd;
		std::string _db;
		std::string _character;
	};
}
