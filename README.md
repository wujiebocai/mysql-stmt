## 环境配置：

1. 使用了c++17特性。
2. mysql支持stmt的版本都可(自己使用的5.7.14版本)。
3. cmake中设置mysql本地相对应路径。

## 使用说明：

​	包含mysql.hpp文件即可使用。

## 简单用法：

1. 查询所有数据：

   ```c++
   #define SELECT_SAMPLE_ALL "SELECT id, name, age FROM student"
   //查询数据库表所有字段数据
   #define SELECT_SAMPLE_ALL "SELECT * FROM student" 
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
   ```

   

2. 查询数据，绑定参数：

   ```c++
   #define SELECT_SAMPLE "SELECT id, name, age FROM student where id like ? or id like ?"
   bool test_stmt_query(mysql_handler& mysql) {
   	auto stmt = mysql.getstmtquery(SELECT_SAMPLE);
   	if (!stmt) {
   		return false;
   	}
   	if (!stmt.query(1, "2")) {//查询id=1和id=2的数据，数据格式随意。
   		return false;
   	}
   	int id = 1;
   	int age = 0;
   	std::string name; name.resize(50);
   
   	stmt.for_each([&](int count) {// 打印所有查询结果， 字段（id, name, age）
   		std::cout << "cccccccccccc:" << count << ",[" << id << "," << name << "," << age << "]" << std::endl;
   	}, id, name, age);
   
   	return true;
   }
   ```

   

3. 查询数据，使用row_ret返回结果：

   ```c++
   #define SELECT_SAMPLE "SELECT id, name, age FROM student where id like ? or id like ?"
   bool test_stmt_query2(mysql_handler& mysql) {
   	auto stmt = mysql.getstmtquery(SELECT_SAMPLE);
   	if (!stmt) {
   		return false;
   	}
   	if (!stmt.query(1, 2)) {//查询id=1和id=2的数据
   		return false;
   	}
   	stmt.get_result([](int count, const ROWSRETPTR& ptr) {//获取所有结果
   		if (!ptr) {
   			return;
   		}
   		auto row_ret = *ptr;
   		int id = row_ret["id"];
   		std::string name = row_ret["name"];
   		int age = row_ret["age"];
   		int cage = row_ret["age"].conv_data<int>();//使用转换数据类型接口获取
   		std::cout << "cccccccccccc:" << count << ",[" << id << "," << name << "," << age << "]" << cage << std::endl;
   	});
   
   	return true;
   }
   ```

   

4. 插入数据：

   ```c++
   #define INSERT_SAMPLE "INSERT INTO \
                          student(id, name, age) \
                          VALUES(?,?,?)"
   bool test_stmt_execute(mysql_handler& mysql) {
   	auto stmt = mysql.getstmtexecute(INSERT_SAMPLE);
   	if (!stmt) {
   		return false;
   	}
   	int id1 = 0;
   	int age1 = 0;
   	std::string name1;
   	//插入三行数据
   	for (int idx = 0; idx < 3; ++idx) {
   		id1++;
   		age1 = id1 + 1;
   		name1 = "www" + std::to_string(id1);
   		auto ret = stmt.execute(id1, name1, age1);
   	}
   	return true;
   }
   ```

注：具体用法和使用细节，请查看具体代码，demo中写了一些用法；里面一些其他小组件的用法看具体实列。