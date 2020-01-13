#pragma once

#include <chrono>
#include <thread>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace mysql_detail {
	//秒
	inline void Sleep_s(int32_t second) {
		std::this_thread::sleep_for(std::chrono::seconds(second));
	}

	//毫秒
	inline void Sleep_m(int32_t milliSeconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(milliSeconds));
	}

	//微秒
	inline void Sleep_w(int32_t microseconds) {
		std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
	}

	//纳秒
	inline void Sleep_n(int32_t nanoseconds) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
	}

	//获取秒数
	inline int64_t GetTimeSeconds() {
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	//获取毫秒数
	inline int64_t GetTimeMillisecond() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	//获取微秒数
	inline int64_t GetTimeMicroseconds() {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	//获取纳秒数
	inline int64_t GetTimeNanosecond() {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	inline int64_t GetUserAccountHashV(const std::string& account) {
		int64_t hashvalue = 0;
		for (unsigned i = 0; i < account.length(); ++i) {
			char c = account.at(i);
			if (c == 0) continue;
			hashvalue += c;
		}

		return hashvalue;
	}
}  // namespace mysql_detail



