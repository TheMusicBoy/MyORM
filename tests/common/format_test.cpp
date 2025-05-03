#include <gtest/gtest.h>
#include <common/format.h>
#include <vector>
#include <map>

using namespace NCommon;

class FormatOptionsTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(FormatOptionsTest, Construction) {
    FormatOptions opts1;
    EXPECT_FALSE(opts1.Has("key"));
    
    FormatOptions opts2("width=10,fill=0,left");
    EXPECT_TRUE(opts2.Has("width"));
    EXPECT_TRUE(opts2.Has("fill"));
    EXPECT_TRUE(opts2.Has("left"));
    EXPECT_EQ(10, opts2.GetInt("width"));
    EXPECT_EQ("0", opts2.GetString("fill"));
    EXPECT_TRUE(opts2.GetBool("left"));
}

TEST_F(FormatOptionsTest, GetSetMethods) {
    FormatOptions opts;
    
    opts.Set("bool_val", true);
    opts.Set("int_val", 42);
    opts.Set("double_val", 3.14159);
    opts.Set("string_val", "hello");
    
    EXPECT_TRUE(opts.GetBool("bool_val"));
    EXPECT_EQ(42, opts.GetInt("int_val"));
    EXPECT_DOUBLE_EQ(3.14159, opts.GetDouble("double_val"));
    EXPECT_EQ("hello", opts.GetString("string_val"));
    
    EXPECT_FALSE(opts.GetBool("non_existent"));
    EXPECT_EQ(100, opts.GetInt("non_existent", 100));
    EXPECT_DOUBLE_EQ(2.5, opts.GetDouble("non_existent", 2.5));
    EXPECT_EQ("default", opts.GetString("non_existent", "default"));
}

TEST_F(FormatOptionsTest, NestedOptions) {
    FormatOptions parent;
    FormatOptions child;
    
    child.Set("nested_value", 42);
    parent.Set("child", child);
    
    const FormatOptions& retrieved = parent.GetOptions("child");
    EXPECT_EQ(42, retrieved.GetInt("nested_value"));
    
    FormatOptions opts("outer={inner=42,flag}");
    const FormatOptions& inner = opts.GetOptions("outer");
    EXPECT_EQ(42, inner.GetInt("inner"));
    EXPECT_TRUE(inner.GetBool("flag"));
}

TEST_F(FormatOptionsTest, SubOptions) {
    FormatOptions opts;
    opts.Set("main.sub1", 1);
    opts.Set("main.sub2", 2);
    opts.Set("main.deep.value", 3);
    opts.Set("other", 4);
    
    FormatOptions subOpts = opts.GetSubOptions("main");
    EXPECT_EQ(1, subOpts.GetInt("sub1"));
    EXPECT_EQ(2, subOpts.GetInt("sub2"));
    EXPECT_EQ(3, subOpts.GetInt("deep.value"));
    EXPECT_FALSE(subOpts.Has("other"));
    
    FormatOptions deepOpts = opts.GetSubOptions("main.deep");
    EXPECT_EQ(3, deepOpts.GetInt("value"));
}

TEST_F(FormatOptionsTest, FormatValueStorage) {
    FormatValue boolVal(true);
    EXPECT_TRUE(boolVal.IsBool());
    EXPECT_TRUE(boolVal.AsBool());
    
    FormatValue intVal(42);
    EXPECT_TRUE(intVal.IsInt());
    EXPECT_EQ(42, intVal.AsInt());
    
    FormatValue doubleVal(3.14159);
    EXPECT_TRUE(doubleVal.IsDouble());
    EXPECT_DOUBLE_EQ(3.14159, doubleVal.AsDouble());
    
    FormatValue stringVal(std::string("hello"));
    EXPECT_TRUE(stringVal.IsString());
    EXPECT_EQ("hello", stringVal.AsString());
    
    FormatOptions opts;
    opts.Set("test", 1);
    FormatValue optsVal(opts);
    EXPECT_TRUE(optsVal.IsOptions());
    EXPECT_EQ(1, optsVal.AsOptions().GetInt("test"));
}

TEST_F(FormatOptionsTest, StringParsing) {
    FormatOptions opts1("key1=value1,key2=42,key3=3.14,key4=true");
    EXPECT_EQ("value1", opts1.GetString("key1"));
    EXPECT_EQ(42, opts1.GetInt("key2"));
    EXPECT_DOUBLE_EQ(3.14, opts1.GetDouble("key3"));
    EXPECT_TRUE(opts1.GetBool("key4"));
    
    FormatOptions opts2("flag1,flag2");
    EXPECT_TRUE(opts2.GetBool("flag1"));
    EXPECT_TRUE(opts2.GetBool("flag2"));
    
    FormatOptions opts3("nested={key1=value1,key2=42}");
    const FormatOptions& nested = opts3.GetOptions("nested");
    EXPECT_EQ("value1", nested.GetString("key1"));
    EXPECT_EQ(42, nested.GetInt("key2"));
    
    FormatOptions opts4("level1={level2={level3=42}}");
    const FormatOptions& level1 = opts4.GetOptions("level1");
    const FormatOptions& level2 = level1.GetOptions("level2");
    EXPECT_EQ(42, level2.GetInt("level3"));
    
    FormatOptions opts5("key1 = value1, key2 = 42");
    EXPECT_EQ("value1", opts5.GetString("key1"));
    EXPECT_EQ(42, opts5.GetInt("key2"));
    
    FormatOptions opts6("key=value with spaces");
    EXPECT_EQ("value with spaces", opts6.GetString("key"));
}

TEST_F(FormatOptionsTest, BooleanFormatting) {
    std::ostringstream out;
    
    FormatHandler(out, true, FormatOptions());
    EXPECT_EQ("true", out.str());
    out.str("");
    
    FormatHandler(out, true, FormatOptions("true=yes,false=no"));
    EXPECT_EQ("yes", out.str());
    out.str("");
    
    FormatHandler(out, false, FormatOptions("true=yes,false=no"));
    EXPECT_EQ("no", out.str());
}

TEST_F(FormatOptionsTest, IntegerFormatting) {
    std::ostringstream out;
    
    FormatHandler(out, 42, FormatOptions());
    EXPECT_EQ("42", out.str());
    out.str("");
    
    FormatHandler(out, 42, FormatOptions("width=4,fill=0"));
    EXPECT_EQ("0042", out.str());
    out.str("");
    
    FormatHandler(out, 42, FormatOptions("width=4,left"));
    EXPECT_EQ("42  ", out.str());
    out.str("");
    
    FormatHandler(out, 42, FormatOptions("base=16"));
    EXPECT_EQ("2a", out.str());
    out.str("");
    
    FormatHandler(out, 42, FormatOptions("base=16,showbase"));
    EXPECT_EQ("0x2a", out.str());
    out.str("");
    
    FormatHandler(out, -42, FormatOptions("width=4,fill=0"));
    EXPECT_EQ("-042", out.str());
}

TEST_F(FormatOptionsTest, DoubleFormatting) {
    std::ostringstream out;
    
    FormatHandler(out, 3.14159, FormatOptions());
    EXPECT_EQ("3.14159", out.str());
    out.str("");
    
    FormatHandler(out, 3.14159, FormatOptions("precision=2"));
    EXPECT_EQ("3.14", out.str());
    out.str("");
    
    FormatHandler(out, 3.14159, FormatOptions("width=6,precision=2"));
    EXPECT_EQ("  3.14", out.str());
    out.str("");
    
    FormatHandler(out, 3.14159, FormatOptions("width=6,precision=2,left"));
    EXPECT_EQ("3.14  ", out.str());
    out.str("");
    
    FormatHandler(out, 3.14159, FormatOptions("width=6,precision=2,fill=0"));
    EXPECT_EQ("003.14", out.str());
}

TEST_F(FormatOptionsTest, StringFormatting) {
    std::ostringstream out;
    
    FormatHandler(out, "hello", FormatOptions());
    EXPECT_EQ("hello", out.str());
    out.str("");
    
    FormatHandler(out, "hello", "width=7");
    EXPECT_EQ("  hello", out.str());
    out.str("");
    
    FormatHandler(out, "hello", "width=7,left");
    EXPECT_EQ("hello  ", out.str());
    out.str("");
    
    FormatHandler(out, "hello", FormatOptions("width=7,fill=*"));
    EXPECT_EQ("**hello", out.str());
    out.str("");
    
    FormatHandler(out, "hello", FormatOptions("maxlength=4"));
    EXPECT_EQ("hell", out.str());
    out.str("");
    
    FormatHandler(out, "Hello", FormatOptions("upper"));
    EXPECT_EQ("HELLO", out.str());
    out.str("");
    
    FormatHandler(out, "Hello", FormatOptions("lower"));
    EXPECT_EQ("hello", out.str());
    out.str("");
    
    FormatHandler(out, "hello", FormatOptions("width=6,upper,maxlength=4"));
    EXPECT_EQ("  HELL", out.str());
}

TEST_F(FormatOptionsTest, VectorFormatting) {
    std::ostringstream out;
    std::vector<int> nums = {1, 2, 3, 4, 5};
    
    FormatHandler(out, nums, FormatOptions());
    EXPECT_EQ("[1, 2, 3, 4, 5]", out.str());
    out.str("");
    
    FormatHandler(out, nums, FormatOptions("delimiter='; ',prefix='(',suffix=')'"));
    EXPECT_EQ("(1; 2; 3; 4; 5)", out.str());
    out.str("");
    
    FormatHandler(out, nums, FormatOptions("limit=3"));
    EXPECT_EQ("[1, 2, 3, ...]", out.str());
    out.str("");
    
    FormatHandler(out, nums, FormatOptions("limit=3,overflow='and 2 more'"));
    EXPECT_EQ("[1, 2, 3, and 2 more]", out.str());
    out.str("");
    
    FormatHandler(out, nums, FormatOptions("element={width=2,fill=0}"));
    EXPECT_EQ("[01, 02, 03, 04, 05]", out.str());
    out.str("");
    
    FormatHandler(out, nums, FormatOptions("delimiter='; ',prefix='(',suffix=')',limit=3,overflow='and 2 more',element={width=2,fill=0}"));
    EXPECT_EQ("(01; 02; 03; and 2 more)", out.str());
}

TEST_F(FormatOptionsTest, MapFormatting) {
    std::ostringstream out;
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Charlie", 92}};
    
    FormatHandler(out, scores, FormatOptions());
    EXPECT_EQ("{Alice: 95, Bob: 87, Charlie: 92}", out.str());
    out.str("");
    
    FormatHandler(out, scores, FormatOptions("delimiter='; ',prefix='[',suffix=']',kv_separator='='"));
    EXPECT_EQ("[Alice=95; Bob=87; Charlie=92]", out.str());
    out.str("");
    
    FormatHandler(out, scores, FormatOptions("limit=2"));
    EXPECT_EQ("{Alice: 95, Bob: 87, ...}", out.str());
    out.str("");
    
    FormatHandler(out, scores, FormatOptions("key={upper},value={width=3,fill=0}"));
    EXPECT_EQ("{ALICE: 095, BOB: 087, CHARLIE: 092}", out.str());
}

TEST_F(FormatOptionsTest, FullFormatFunction) {
    EXPECT_EQ("Answer is 42", Format("Answer is {}", 42));
    EXPECT_EQ("Pi is 3.14", Format("Pi is {precision=2}", 3.14159));
    EXPECT_EQ("Name: JOHN", Format("Name: {upper}", "John"));
    
    std::vector<int> nums = {1, 2, 3};
    EXPECT_EQ("Numbers: [01, 02, 03]", Format("Numbers: {element={width=2,fill=0}}", nums));
    
    EXPECT_EQ("Test: 42, 3.14, true", 
              Format("Test: {}, {precision=2}, {true=true,false=false}", 42, 3.14159, true));
              
    std::map<std::string, std::vector<int>> data = {
        {"scores", {90, 85, 95}}
    };
    EXPECT_EQ("Results: {scores: [90, 85, 95]}", 
              Format("Results: {}", data));
}

TEST_F(FormatOptionsTest, EdgeCases) {
    FormatOptions opts2("key=,=value");
    EXPECT_TRUE(opts2.Has("key"));
    EXPECT_EQ("", opts2.GetString("key"));
    
    FormatOptions opts3("key=1,key=2");
    
    FormatOptions opts4("nested={unclosed");
    EXPECT_TRUE(opts4.Has("nested"));
    EXPECT_EQ("{unclosed", opts4.GetString("nested"));
    
    FormatOptions opts5("key=");
    EXPECT_TRUE(opts5.Has("key"));
    EXPECT_EQ("", opts5.GetString("key"));
}

class FormatTest : public ::testing::Test {
protected:
    template<typename... Args>
    std::string TestFormat(const std::string& format, Args&&... args) {
        return Format(format, std::forward<Args>(args)...);
    }
};

TEST_F(FormatTest, BooleanFormatting) {
    EXPECT_EQ("true", TestFormat("{}", true));
    EXPECT_EQ("false", TestFormat("{}", false));
    
    EXPECT_EQ("yes", TestFormat("{true=yes,false=no}", true));
    EXPECT_EQ("no", TestFormat("{true=yes,false=no}", false));
    
    EXPECT_EQ("✓", TestFormat("{true=✓,false=✗}", true));
    EXPECT_EQ("✗", TestFormat("{true=✓,false=✗}", false));
}

TEST_F(FormatTest, IntegerFormatting) {
    EXPECT_EQ("42", TestFormat("{}", 42));
    
    EXPECT_EQ("  42", TestFormat("{width=4}", 42));
    EXPECT_EQ("42  ", TestFormat("{width=4,left}", 42));
    EXPECT_EQ("0042", TestFormat("{width=4,fill=0}", 42));
    
    EXPECT_EQ("2a", TestFormat("{base=16}", 42));
    EXPECT_EQ("0x2a", TestFormat("{base=16,showbase}", 42));
    EXPECT_EQ("52", TestFormat("{base=8}", 42));
    
    EXPECT_EQ("-42", TestFormat("{}", -42));
    EXPECT_EQ("-042", TestFormat("{width=4,fill=0}", -42));
}

TEST_F(FormatTest, FloatFormatting) {
    EXPECT_EQ("3.14159", TestFormat("{}", 3.14159));
    
    EXPECT_EQ("3.14", TestFormat("{precision=2}", 3.14159));
    EXPECT_EQ("3.142", TestFormat("{precision=3}", 3.14159));
    
    EXPECT_EQ("  3.14", TestFormat("{width=6,precision=2}", 3.14159));
    EXPECT_EQ("3.14  ", TestFormat("{width=6,precision=2,left}", 3.14159));
    EXPECT_EQ("003.14", TestFormat("{width=6,precision=2,fill=0}", 3.14159));
    
    EXPECT_EQ("3.142e+00", TestFormat("{precision=3,scientific}", 3.14159));
    
    EXPECT_EQ("3.142", TestFormat("{precision=3,fixed}", 3.14159));
}

TEST_F(FormatTest, StringFormatting) {
    EXPECT_EQ("hello", TestFormat("{}", "hello"));
    
    EXPECT_EQ("  hello", TestFormat("{width=7}", "hello"));
    EXPECT_EQ("hello  ", TestFormat("{width=7,left}", "hello"));
    EXPECT_EQ("**hello", TestFormat("{width=7,fill=*}", "hello"));
    
    EXPECT_EQ("hello", TestFormat("{maxlength=10}", "hello"));
    EXPECT_EQ("hell", TestFormat("{maxlength=4}", "hello"));
    
    EXPECT_EQ("HELLO", TestFormat("{upper}", "Hello"));
    EXPECT_EQ("hello", TestFormat("{lower}", "Hello"));
    
    EXPECT_EQ("  HELL", TestFormat("{width=6,upper,maxlength=4}", "hello"));
}

TEST_F(FormatTest, VectorFormatting) {
    std::vector<int> nums = {1, 2, 3, 4, 5};
    
    EXPECT_EQ("[1, 2, 3, 4, 5]", TestFormat("{}", nums));
    
    EXPECT_EQ("(1; 2; 3; 4; 5)", TestFormat("{delimiter='; ',prefix='(',suffix=')'}", nums));
    
    EXPECT_EQ("[1, 2, 3, ...]", TestFormat("{limit=3}", nums));
    EXPECT_EQ("[1, 2, 3, and 2 more]", TestFormat("{limit=3,overflow= and 2 more}", nums));
    
    EXPECT_EQ("[01, 02, 03, 04, 05]", TestFormat("{element={width=2,fill=0}}", nums));
    
    EXPECT_EQ("(01; 02; 03; and 2 more)", 
              TestFormat("{delimiter='; ',prefix='(',suffix=')',limit=3,overflow='and 2 more',element={width=2,fill=0}}", nums));
}

TEST_F(FormatTest, ListFormatting) {
    std::list<int> values = {1, 2, 3, 4, 5};
    
    EXPECT_EQ("[1, 2, 3, 4, 5]", TestFormat("{}", values));
    EXPECT_EQ("(1; 2; 3; 4; 5)", TestFormat("{delimiter='; ',prefix='(',suffix=')'}", values));
    EXPECT_EQ("[1, 2, 3, ...]", TestFormat("{limit=3}", values));
    EXPECT_EQ("[01, 02, 03, 04, 05]", TestFormat("{element={width=2,fill=0}}", values));
}

TEST_F(FormatTest, DequeFormatting) {
    std::deque<int> values = {1, 2, 3, 4, 5};
    
    EXPECT_EQ("[1, 2, 3, 4, 5]", TestFormat("{}", values));
    EXPECT_EQ("(1; 2; 3; 4; 5)", TestFormat("{delimiter='; ',prefix='(',suffix=')'}", values));
    EXPECT_EQ("[1, 2, 3, ...]", TestFormat("{limit=3}", values));
    EXPECT_EQ("[01, 02, 03, 04, 05]", TestFormat("{element={width=2,fill=0}}", values));
}

TEST_F(FormatTest, SetFormatting) {
    std::set<int> values = {5, 3, 1, 4, 2};
    
    EXPECT_EQ("[1, 2, 3, 4, 5]", TestFormat("{}", values));
    EXPECT_EQ("(1; 2; 3; 4; 5)", TestFormat("{delimiter='; ',prefix='(',suffix=')'}", values));
    EXPECT_EQ("[1, 2, 3, ...]", TestFormat("{limit=3}", values));
    EXPECT_EQ("[01, 02, 03, 04, 05]", TestFormat("{element={width=2,fill=0}}", values));
}

TEST_F(FormatTest, UnorderedSetFormatting) {
    std::unordered_set<int> values = {5, 3, 1, 4, 2};
    
    std::string result = TestFormat("{}", values);
    EXPECT_TRUE(result.find("1") != std::string::npos);
    EXPECT_TRUE(result.find("2") != std::string::npos);
    EXPECT_TRUE(result.find("3") != std::string::npos);
    EXPECT_TRUE(result.find("4") != std::string::npos);
    EXPECT_TRUE(result.find("5") != std::string::npos);
    
    result = TestFormat("{element={width=2,fill=0}}", values);
    EXPECT_TRUE(result.find("01") != std::string::npos);
    EXPECT_TRUE(result.find("02") != std::string::npos);
    EXPECT_TRUE(result.find("03") != std::string::npos);
    EXPECT_TRUE(result.find("04") != std::string::npos);
    EXPECT_TRUE(result.find("05") != std::string::npos);
}

TEST_F(FormatTest, ArrayFormatting) {
    std::array<int, 5> values = {1, 2, 3, 4, 5};
    
    EXPECT_EQ("[1, 2, 3, 4, 5]", TestFormat("{}", values));
    EXPECT_EQ("(1; 2; 3; 4; 5)", TestFormat("{delimiter='; ',prefix='(',suffix=')'}", values));
    EXPECT_EQ("[1, 2, 3, ...]", TestFormat("{limit=3}", values));
    EXPECT_EQ("[01, 02, 03, 04, 05]", TestFormat("{element={width=2,fill=0}}", values));
}

TEST_F(FormatTest, MapFormatting) {
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Charlie", 92}};
    
    EXPECT_EQ("{Alice: 95, Bob: 87, Charlie: 92}", TestFormat("{}", scores));
    
    EXPECT_EQ("[Alice=95; Bob=87; Charlie=92]", 
              TestFormat("{prefix=[,suffix=],delimiter='; ',kv_separator='='}", scores));
    
    EXPECT_EQ("{Alice: 95, Bob: 87, ...}", TestFormat("{limit=2}", scores));
    
    EXPECT_EQ("{ALICE: 095, BOB: 087, CHARLIE: 092}", 
              TestFormat("{key={upper},value={width=3,fill=0}}", scores));
}

TEST_F(FormatTest, UnorderedMapFormatting) {
    std::unordered_map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Charlie", 92}};
    
    std::string result = TestFormat("{}", scores);
    EXPECT_TRUE(result.find("Alice: 95") != std::string::npos);
    EXPECT_TRUE(result.find("Bob: 87") != std::string::npos);
    EXPECT_TRUE(result.find("Charlie: 92") != std::string::npos);
    
    result = TestFormat("{key={upper},value={width=3,fill=0}}", scores);
    EXPECT_TRUE(result.find("ALICE: 095") != std::string::npos);
    EXPECT_TRUE(result.find("BOB: 087") != std::string::npos);
    EXPECT_TRUE(result.find("CHARLIE: 092") != std::string::npos);
}

TEST_F(FormatTest, AdvancedNestedContainers) {
    std::vector<std::list<int>> nestedList = {{1, 2, 3}, {4, 5, 6}};
    EXPECT_EQ("[[1, 2, 3], [4, 5, 6]]", TestFormat("{}", nestedList));
    
    std::map<std::string, std::set<int>> mapOfSets = {
        {"even", {2, 4, 6, 8}},
        {"odd", {1, 3, 5, 7}}
    };
    std::string result = TestFormat("{}", mapOfSets);
    EXPECT_TRUE(result.find("even: [2, 4, 6, 8]") != std::string::npos);
    EXPECT_TRUE(result.find("odd: [1, 3, 5, 7]") != std::string::npos);
    
    result = TestFormat("{value={element={width=2,fill=0}}}", mapOfSets);
    EXPECT_TRUE(result.find("even: [02, 04, 06, 08]") != std::string::npos);
    EXPECT_TRUE(result.find("odd: [01, 03, 05, 07]") != std::string::npos);
}

TEST_F(FormatTest, ExceptionFormatting) {
    try {
        throw std::runtime_error("Test error message");
    } catch (const std::exception& e) {
        EXPECT_EQ("Test error message", TestFormat("{}", e));
        EXPECT_EQ("ERROR: Test error message", TestFormat("ERROR: {}", e));
        EXPECT_EQ("TEST ERROR MESSAGE", TestFormat("{upper}", e));
    }
}

TEST_F(FormatTest, MixedContainerTypes) {
    std::pair<std::string, std::vector<int>> pair = {"numbers", {1, 2, 3}};
    std::string result = TestFormat("{}", pair);
    EXPECT_EQ("(numbers, [1, 2, 3])", result);
    
    std::vector<std::pair<int, std::string>> vecOfPairs = {{1, "one"}, {2, "two"}, {3, "three"}};
    result = TestFormat("{}", vecOfPairs);
    EXPECT_EQ("[(1, one), (2, two), (3, three)]", result);
    
    result = TestFormat("{element={delimiter=' - ',prefix='<',suffix='>'}}", vecOfPairs);
    EXPECT_EQ("[<1 - one>, <2 - two>, <3 - three>]", result);
}

TEST_F(FormatTest, EmptyContainers) {
    std::list<int> emptyList;
    EXPECT_EQ("[]", TestFormat("{}", emptyList));
    
    std::deque<int> emptyDeque;
    EXPECT_EQ("[]", TestFormat("{}", emptyDeque));
    
    std::set<int> emptySet;
    EXPECT_EQ("[]", TestFormat("{}", emptySet));
    
    std::unordered_set<int> emptyUSet;
    EXPECT_EQ("[]", TestFormat("{}", emptyUSet));
    
    std::unordered_map<std::string, int> emptyUMap;
    EXPECT_EQ("{}", TestFormat("{}", emptyUMap));
    
    std::array<int, 0> emptyArray = {};
    EXPECT_EQ("[]", TestFormat("{}", emptyArray));
}

TEST_F(FormatTest, NestedOptionsFormatting) {
    std::vector<std::map<std::string, int>> data = {
        {{"x", 1}, {"y", 2}},
        {{"x", 3}, {"y", 4}}
    };
    
    EXPECT_EQ("[{x: 1, y: 2}, {x: 3, y: 4}]", TestFormat("{}", data));
    
    EXPECT_EQ("[{x: 01, y: 02}, {x: 03, y: 04}]", 
              TestFormat("{element={value={width=2,fill=0}}}", data));
    
    EXPECT_EQ("[(X=1, Y=2); (X=3, Y=4)]", 
              TestFormat("{prefix=[,suffix=],delimiter='; ',element={prefix=(,suffix=),kv_separator==,key={upper}}}", data));
}

TEST_F(FormatTest, FormatOptionsParsing) {
    FormatOptions opts1("width=10,left,fill=0");
    EXPECT_EQ(10, opts1.GetInt("width"));
    EXPECT_TRUE(opts1.GetBool("left"));
    EXPECT_EQ("0", opts1.GetString("fill"));
    
    FormatOptions opts2("container={prefix=[,suffix=],element={width=2,fill=0}}");
    FormatOptions containerOpts = opts2.GetOptions("container");
    EXPECT_EQ("[", containerOpts.GetString("prefix"));
    EXPECT_EQ("]", containerOpts.GetString("suffix"));
    
    FormatOptions elementOpts = containerOpts.GetOptions("element");
    EXPECT_EQ(2, elementOpts.GetInt("width"));
    EXPECT_EQ("0", elementOpts.GetString("fill"));
    
    FormatOptions opts3("a={b={c={d=value}}}");
    EXPECT_EQ("value", opts3.GetOptions("a").GetOptions("b").GetOptions("c").GetString("d"));
}

TEST_F(FormatTest, EdgeCases) {
    EXPECT_EQ("", TestFormat("{}", ""));
    
    std::vector<int> emptyVec;
    EXPECT_EQ("[]", TestFormat("{}", emptyVec));
    
    std::map<std::string, int> emptyMap;
    EXPECT_EQ("{}", TestFormat("{}", emptyMap));
    
    EXPECT_NO_THROW(FormatOptions("{unclosed={nested}"));
    
    EXPECT_EQ("Hello 42 3.14 true", TestFormat("Hello {} {} {}", 42, 3.14, true));
}

TEST_F(FormatTest, IndexedPlaceholders) {
    EXPECT_EQ("Hello, World!", TestFormat("Hello, $1{}!", "World"));
    EXPECT_EQ("Value: 42", TestFormat("Value: $1{}", 42));
    
    EXPECT_EQ("Second: 2, First: 1", TestFormat("Second: $2{}, First: $1{}", 1, 2));
    EXPECT_EQ("3, 1, 2", TestFormat("$3{}, $1{}, $2{}", 1, 2, 3));
    
    EXPECT_EQ("1, 1, 1", TestFormat("$1{}, $1{}, $1{}", 1, 2, 3));
    
    EXPECT_EQ("  42", TestFormat("$1{width=4}", 42));
    EXPECT_EQ("0042", TestFormat("$1{width=4,fill=0}", 42));
    
    EXPECT_EQ("1, 1, 3", TestFormat("$1{}, {}, $3{}", 1, 2, 3));
    
    EXPECT_EQ("NAME: JOHN, ID: 007", 
              TestFormat("NAME: $1{upper}, ID: $2{width=3,fill=0}", "John", 7));
    
    std::vector<int> nums = {1, 2, 3};
    EXPECT_EQ("Numbers: [01, 02, 03]", 
              TestFormat("Numbers: $1{element={width=2,fill=0}}", nums));
}

TEST_F(FormatTest, IndexedPlaceholdersEdgeCases) {
    EXPECT_EQ("", TestFormat("$4{}", 1, 2, 3));
    
    EXPECT_EQ("$a1", TestFormat("$a{}", 1, 2, 3));
    
    EXPECT_EQ("1", TestFormat("$1{}", 1));
    
    EXPECT_EQ("10th arg", TestFormat("$10{} arg", 1, 2, 3, 4, 5, 6, 7, 8, 9, "10th"));
    
    std::map<std::string, int> map1 = {{"a", 1}};
    std::map<std::string, int> map2 = {{"b", 2}};
    EXPECT_EQ("Map1: {a: 1}, Map2: {b: 2}", 
              TestFormat("Map1: $1{}, Map2: $2{}", map1, map2));
}

TEST_F(FormatTest, IndexedVsSequentialPerformance) {
    std::string result1, result2;
    
    result1 = TestFormat("{} {} {} {} {}", 1, 2, 3, 4, 5);
    EXPECT_EQ("1 2 3 4 5", result1);
    
    result2 = TestFormat("$1{} $2{} $3{} $4{} $5{}", 1, 2, 3, 4, 5);
    EXPECT_EQ("1 2 3 4 5", result2);
    
    result2 = TestFormat("$5{} $4{} $3{} $2{} $1{}", 1, 2, 3, 4, 5);
    EXPECT_EQ("5 4 3 2 1", result2);
}

TEST_F(FormatTest, ChronoTimePointFormatting) {
    std::tm tm{};
    tm.tm_year = 123;
    tm.tm_mon = 4;
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 45;
    std::time_t time = std::mktime(&tm) - timezone;
    auto systemTimePoint = std::chrono::system_clock::from_time_t(time);
    
    auto systemTimePointWithMicros = systemTimePoint + std::chrono::microseconds(123456);
    
    EXPECT_EQ("2023-05-15T10:30:45Z", TestFormat("{}", systemTimePoint));
    
    EXPECT_EQ("2023-05-15T10:30:45.1Z", TestFormat("{precision=1}", systemTimePointWithMicros));
    EXPECT_EQ("2023-05-15T10:30:45.123Z", TestFormat("{precision=3}", systemTimePointWithMicros));
    EXPECT_EQ("2023-05-15T10:30:45.123456Z", TestFormat("{precision=6}", systemTimePointWithMicros));
    
    EXPECT_EQ("2023-05-15T13:30:45", TestFormat("{local}", systemTimePoint));
    EXPECT_EQ("2023-05-15T13:30:45.123", TestFormat("{local,precision=3}", systemTimePointWithMicros));
    
    EXPECT_EQ("2023-05-15 10:30:45Z", TestFormat("{format=rfc3339}", systemTimePoint));
    EXPECT_EQ("2023-05-15 10:30:45.123Z", TestFormat("{format=rfc3339,precision=3}", systemTimePointWithMicros));
    
    int64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        systemTimePoint.time_since_epoch()).count();
    EXPECT_EQ(std::to_string(timestamp), TestFormat("{format=timestamp}", systemTimePoint));
    
    std::string timestampWithPrecision = std::to_string(timestamp) + ".123";
    EXPECT_EQ(timestampWithPrecision, TestFormat("{format=timestamp,precision=3}", systemTimePointWithMicros));
    
    EXPECT_EQ("2023-05-15", TestFormat("{format=custom,strftime=%Y-%m-%d}", systemTimePoint));
    EXPECT_EQ("10:30:45", TestFormat("{format=custom,strftime=%H:%M:%S}", systemTimePoint));
    EXPECT_EQ("Monday, May", TestFormat("{format=custom,strftime='%A, %B'}", systemTimePoint));
    
    EXPECT_EQ("2023-05-15 - 10:30:45.123", 
        TestFormat("{format=custom,strftime=%Y-%m-%d - %H:%M:%S,precision=3}", systemTimePointWithMicros));
    
    std::chrono::steady_clock::time_point steadyTimePoint = 
        std::chrono::steady_clock::time_point(std::chrono::seconds(12345) + std::chrono::microseconds(678910));
    
    EXPECT_EQ("12345s", TestFormat("{}", steadyTimePoint));
    EXPECT_EQ("12345s.678", TestFormat("{precision=3}", steadyTimePoint));
    EXPECT_EQ("12345678ms", TestFormat("{unit=ms}", steadyTimePoint));
    EXPECT_EQ("12345678910μs", TestFormat("{unit=μs}", steadyTimePoint));
    EXPECT_EQ("12345678910000ns", TestFormat("{unit=ns}", steadyTimePoint));
    EXPECT_EQ("205m", TestFormat("{unit=m}", steadyTimePoint));
    EXPECT_EQ("3h", TestFormat("{unit=h}", steadyTimePoint));
}

TEST_F(FormatTest, ChronoTimePointWithPlaceholders) {
    std::tm tm{};
    tm.tm_year = 123;
    tm.tm_mon = 4;
    tm.tm_mday = 15;
    tm.tm_hour = 10;
    tm.tm_min = 30;
    tm.tm_sec = 45;
    std::time_t time = std::mktime(&tm) - timezone;
    auto systemTimePoint = std::chrono::system_clock::from_time_t(time);
    
    EXPECT_EQ("Time: 2023-05-15T10:30:45Z", 
        TestFormat("Time: {}", systemTimePoint));
    
    EXPECT_EQ("ISO: 2023-05-15T10:30:45Z, Custom: 15/05/2023", 
        TestFormat("ISO: $1{}, Custom: $1{format=custom,strftime=%d/%m/%Y}", systemTimePoint));
    
    EXPECT_EQ("Date: 2023-05-15, Time: 10:30:45", 
        TestFormat("Date: {format=custom,strftime=%Y-%m-%d}, Time: {format=custom,strftime=%H:%M:%S}", 
                  systemTimePoint, systemTimePoint));
    
    auto timeWithMs = systemTimePoint + std::chrono::milliseconds(123);
    EXPECT_EQ("Time with ms: 2023-05-15T10:30:45.123Z", 
        TestFormat("Time with ms: {precision=3}", timeWithMs));
    
    EXPECT_EQ("User 'John' logged in at 2023-05-15T10:30:45Z with status: active", 
        TestFormat("User \\'{}\\' logged in at {} with status: {}", 
                  "John", systemTimePoint, "active"));
}

TEST_F(FormatTest, BasicEscaping) {
    EXPECT_EQ("Text with a single quote '", TestFormat("Text with a single quote \\'"));
    EXPECT_EQ("Text with a backslash \\", TestFormat("Text with a backslash \\\\"));
    EXPECT_EQ("Text with newline \n", TestFormat("Text with newline \\n"));
}

TEST_F(FormatTest, EscapingWithPlaceholders) {
    EXPECT_EQ("Value in 'quotes': 42", TestFormat("Value in \\'quotes\\': {}", 42));
    
    EXPECT_EQ("Value: 'hello'", TestFormat("Value: \\'{}\\'", "hello"));
    
    EXPECT_EQ("Hello 'World'!", TestFormat("Hello \\'$1{}\\'!", "World"));
}

TEST_F(FormatTest, EscapingBraces) {
    EXPECT_EQ("Text with braces {}", TestFormat("Text with braces \\{\\}"));
    
    EXPECT_EQ("Format: {} Value: 42", TestFormat("Format: \\{\\} Value: {}", 42));
    
    EXPECT_EQ("Nested {braces} example", TestFormat("Nested \\{braces\\} example"));
}

TEST_F(FormatTest, PlaceholderWithEscaping) {
    EXPECT_EQ("Testing 'hello'", TestFormat("Testing \\'{}\\'", "hello"));
    
    EXPECT_EQ("Path: C:\\Windows\\System32", 
              TestFormat("Path: {}", "C:\\Windows\\System32"));
}
