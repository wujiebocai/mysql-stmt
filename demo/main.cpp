#include <iostream>
#include <ctime>
#include <sstream>

#include <mysql.hpp>
#include <util/allocator.hpp>

#define STRING_SIZE 50
#define SELECT_SAMPLE_ALL "SELECT id, name, age FROM student"
// 测试stmt查询所有数据
bool test_stmt_query_all(mysql_handler& mysql) {
	auto stmt = mysql.getstmtquery(SELECT_SAMPLE_ALL);
	if (!stmt) {
		return false;
	}

	if (!stmt.query()) {
		return false;
	}

	int id = 1;
	int age = 0;
	std::string name; name.resize(50);

	stmt.for_each([&](int count) { // 打印所有查询结果， 字段（id, name, age）
		std::cout << "cccccccccccc:" << count << ",[" << id << "," << name << "," << age << "]" << std::endl;
	}, id, name, age);

	return true;
}

#define SELECT_SAMPLE "SELECT id, name, age FROM student where id like ? or id like ?"
//也可以使用下面格式
//#define SELECT_SAMPLE "SELECT * FROM student where id like ?"
// 测试stmt查询符合条件数据
bool test_stmt_query(mysql_handler& mysql) {
	auto stmt = mysql.getstmtquery(SELECT_SAMPLE);
	if (!stmt) {
		return false;
	}

	if (!stmt.query(1, 2)) {
		return false;
	}

	int id = 1;
	int age = 0;
	std::string name; name.resize(50);

	stmt.for_each([&](int count) { // 打印所有查询结果， 字段（id, name, age）
		std::cout << "cccccccccccc:" << count << ",[" << id << "," << name << "," << age << "]" << std::endl;
	}, id, name, age);

	return true;
}

bool test_stmt_query2(mysql_handler& mysql) {
	auto stmt = mysql.getstmtquery(SELECT_SAMPLE);
	if (!stmt) {
		return false;
	}

	if (!stmt.query(1, 2)) {
		return false;
	}

	stmt.get_result([](int count, const ROWSRETPTR& ptr) {
		if (!ptr) {
			return;
		}
		auto row_ret = *ptr;
		int id = row_ret["id"];
		std::string name = row_ret["name"];
		int age = row_ret["age"];
		int cage = row_ret["age"].conv_data<int>();
		std::cout << "cccccccccccc:" << count << ",[" << id << "," << name << "," << age << "]" << cage << std::endl;
	});

	return true;
}

#define INSERT_SAMPLE "INSERT INTO \
                       student(id, name, age) \
                       VALUES(?,?,?)"

// 测试stmt插入数据
bool test_stmt_execute(mysql_handler& mysql) {
	auto stmt = mysql.getstmtexecute(INSERT_SAMPLE);
	if (!stmt) {
		return false;
	}
	int id1 = 0;
	int age1 = 0;
	std::string name1;
	//插入三行数据
	for (int idx = 0; idx <= 3; ++idx) {
		id1++;
		age1 = id1 + 1;
		name1 = "www" + std::to_string(id1);
		auto ret = stmt.execute(id1, name1, age1);
		std::cout << "fffffffffffffff:" << ret << std::endl;
	}
	return true;
}

//测试自定义内存分配器：allocator_m
template<class K, class T>
struct test_map_use_alloc {
	typedef std::unordered_map<K, T, std::hash<K>, std::equal_to<K>, allocator_m<std::pair<const K, T> > > TESTMAPTYPE;

	test_map_use_alloc() = default;
	~test_map_use_alloc() = default;

	bool add(K k, T t) {
		bool inserted = this->tsdata_.try_emplace(k, t).second;
		return (inserted ? 1 : 0);
	}
	void print_data() {
		for (auto& item : tsdata_) {
			std::cout << "test_map_use_alloc:[" << item.first << ", " << item.second << "]" << std::endl;
		}
	}
	void reset() {
		tsdata_.clear();
	}
private:
	TESTMAPTYPE tsdata_;
};
void test_stmt_allocator_m() {
	//测试分配一个整数
	allocator_m<int> a1;
	int* a = a1.allocate(1);
	a1.construct(a, 7);
	std::cout << a[0] << '\n';
	a1.deallocate(a, 1); //释放内存

	// 测试分配一个字符串数组
	//decltype(a1)::rebind<std::string>::other a2;
	std::allocator_traits<decltype(a1)>::rebind_alloc<std::string> a2;
	std::string* s = a2.allocate(2); // 2 个 string 的空间
	a2.construct(s, "foo");
	a2.construct(s + 1, "bar");
	std::cout << s[0] << ' ' << s[1] << '\n';
	a2.destroy(s);
	a2.destroy(s + 1);
	a2.deallocate(s, 2);

	// 测试vector，使用allocator_m
	std::vector<std::string, allocator_m<std::string> > v;
	v.push_back("aaaaaaaaaa");
	v.push_back("cccccccccc");
	v.push_back("vvvvvvv");
	v.push_back("aaaaaa");
	v.push_back("wwwwwwwwwww");
	//v.clear();
	for (std::size_t i = 0; i < v.size(); ++i) {
		std::cout << "vvvvvvvvvv:" << v[i] << std::endl;
	}
	v.clear();

	//测试map使用allocator_m分配器
	test_map_use_alloc<std::string, std::string> mapdata;
	mapdata.add("1", "aaaaaaaaaa");
	mapdata.add("2", "cccccccaaaaaaaaaaaa");
	mapdata.add("3", "bbbbbbbaaaaaaaaa");
	mapdata.add("4", "gggggggaaaaaaaaaa");
	mapdata.print_data();
	mapdata.reset();
}

int main(int argc, char* argv[]) {
	std::string host("127.0.0.1");
	unsigned int port = 3306;
	std::string user("root");
	std::string password("");
	std::string schema("async_mysql_test");
	std::string character("utf8");
	mysql_handler mysqlhandler(host, port, user, password, schema, character);
	
	if (!mysqlhandler.connect()) {
		return 0;
	}

	//测试插入数据
	test_stmt_execute(mysqlhandler);
	//测试查询数据
	test_stmt_query2(mysqlhandler);

	//测试自定义内存分配器
	test_stmt_allocator_m();

	std::string str;
	std::cin >> str;
	return 0;
}
