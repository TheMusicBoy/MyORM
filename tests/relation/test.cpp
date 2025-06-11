#include <gtest/gtest.h>
#include <relation/path.h>

namespace NOrm::NRelation::Tests {

class TMessagePathTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create some test entries
        entry1_ = {1, "field1"};
        entry2_ = {2, "field2"};
        entry3_ = {3, "field3"};
    }

    TMessagePathEntry entry1_;
    TMessagePathEntry entry2_;
    TMessagePathEntry entry3_;
};

// Test constructors
TEST_F(TMessagePathTest, DefaultConstructor) {
    TMessagePath path;
    EXPECT_TRUE(path.empty());
    EXPECT_EQ(path.data().size(), 0);
}

TEST_F(TMessagePathTest, EntryConstructor) {
    TMessagePath path(entry1_);
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path.data().size(), 1);
    EXPECT_EQ(path.at(0).protonum, 1);
    EXPECT_EQ(path.at(0).name, "field1");
}

TEST_F(TMessagePathTest, VectorConstructor) {
    std::vector<TMessagePathEntry> entries = {entry1_, entry2_};
    TMessagePath path(entries);
    EXPECT_EQ(path.data().size(), 2);
    EXPECT_EQ(path.at(0).protonum, 1);
    EXPECT_EQ(path.at(1).protonum, 2);
}

TEST_F(TMessagePathTest, CopyConstructor) {
    TMessagePath original({entry1_, entry2_});
    TMessagePath copy(original);
    EXPECT_EQ(copy.data().size(), 2);
    EXPECT_EQ(copy.at(0).protonum, 1);
    EXPECT_EQ(copy.at(1).protonum, 2);
}

TEST_F(TMessagePathTest, MoveConstructor) {
    TMessagePath original({entry1_, entry2_});
    TMessagePath moved(std::move(original));
    EXPECT_EQ(moved.data().size(), 2);
    EXPECT_EQ(moved.at(0).protonum, 1);
    EXPECT_EQ(moved.at(1).protonum, 2);
}

// Test assignment operators
TEST_F(TMessagePathTest, CopyAssignment) {
    TMessagePath original({entry1_, entry2_});
    TMessagePath copy;
    copy = original;
    EXPECT_EQ(copy.data().size(), 2);
    EXPECT_EQ(copy.at(0).protonum, 1);
    EXPECT_EQ(copy.at(1).protonum, 2);
}

TEST_F(TMessagePathTest, MoveAssignment) {
    TMessagePath original({entry1_, entry2_});
    TMessagePath moved;
    moved = std::move(original);
    EXPECT_EQ(moved.data().size(), 2);
    EXPECT_EQ(moved.at(0).protonum, 1);
    EXPECT_EQ(moved.at(1).protonum, 2);
}

// Test concatenation operators
TEST_F(TMessagePathTest, AppendPath) {
    TMessagePath path1({entry1_});
    TMessagePath path2({entry2_});
    path1 /= path2;
    EXPECT_EQ(path1.data().size(), 2);
    EXPECT_EQ(path1.at(0).protonum, 1);
    EXPECT_EQ(path1.at(1).protonum, 2);
}

TEST_F(TMessagePathTest, AppendEntry) {
    TMessagePath path({entry1_});
    path /= entry2_;
    EXPECT_EQ(path.data().size(), 2);
    EXPECT_EQ(path.at(0).protonum, 1);
    EXPECT_EQ(path.at(1).protonum, 2);
}

TEST_F(TMessagePathTest, ConcatenatePath) {
    TMessagePath path1({entry1_});
    TMessagePath path2({entry2_});
    TMessagePath result = path1 / path2;
    EXPECT_EQ(result.data().size(), 2);
    EXPECT_EQ(result.at(0).protonum, 1);
    EXPECT_EQ(result.at(1).protonum, 2);
    // Original paths should remain unchanged
    EXPECT_EQ(path1.data().size(), 1);
    EXPECT_EQ(path2.data().size(), 1);
}

// Test parent method
TEST_F(TMessagePathTest, Parent) {
    TMessagePath path({entry1_, entry2_, entry3_});
    TMessagePath parent = path.parent();
    EXPECT_EQ(parent.data().size(), 2);
    EXPECT_EQ(parent.at(0).protonum, 1);
    EXPECT_EQ(parent.at(1).protonum, 2);
}

TEST_F(TMessagePathTest, ParentOfEmptyPath) {
    TMessagePath path;
    TMessagePath parent = path.parent();
    EXPECT_TRUE(parent.empty());
}

// Test comparison operators
TEST_F(TMessagePathTest, EqualityOperator) {
    TMessagePath path1({entry1_, entry2_});
    TMessagePath path2({entry1_, entry2_});
    TMessagePath path3({entry1_, entry3_});
    
    EXPECT_TRUE(path1 == path2);
    EXPECT_FALSE(path1 == path3);
}

TEST_F(TMessagePathTest, InequalityOperator) {
    TMessagePath path1({entry1_, entry2_});
    TMessagePath path2({entry1_, entry2_});
    TMessagePath path3({entry1_, entry3_});
    
    EXPECT_FALSE(path1 != path2);
    EXPECT_TRUE(path1 != path3);
}

TEST_F(TMessagePathTest, LessThanOperator) {
    TMessagePath path1({entry1_, entry2_});
    TMessagePath path2({entry1_, entry3_}); // entry3_.protonum > entry2_.protonum
    TMessagePath path3({entry1_});          // Shorter path
    
    EXPECT_TRUE(path1 < path2);    // Same length, but path1 has smaller protonum
    EXPECT_FALSE(path2 < path1);   // Same length, but path2 has larger protonum
    EXPECT_TRUE(path3 < path1);    // path3 is shorter
    EXPECT_FALSE(path1 < path3);   // path1 is longer
}

// Test path relationship methods
TEST_F(TMessagePathTest, IsParentOf) {
    TMessagePath parent({entry1_});
    TMessagePath child({entry1_, entry2_});
    TMessagePath unrelated({entry3_});
    TMessagePath deeper({entry1_, entry2_, entry3_});
    
    EXPECT_TRUE(parent.isParentOf(child));
    EXPECT_FALSE(parent.isParentOf(parent));      // Not parent of self
    EXPECT_FALSE(parent.isParentOf(unrelated));   // Unrelated paths
    EXPECT_FALSE(parent.isParentOf(deeper));      // Ancestor but not direct parent
}

TEST_F(TMessagePathTest, IsAncestorOf) {
    TMessagePath ancestor({entry1_});
    TMessagePath parent({entry1_, entry2_});
    TMessagePath child({entry1_, entry2_, entry3_});
    TMessagePath unrelated({entry3_});
    
    EXPECT_TRUE(ancestor.isAncestorOf(parent));
    EXPECT_TRUE(ancestor.isAncestorOf(child));
    EXPECT_FALSE(ancestor.isAncestorOf(ancestor)); // Not ancestor of self
    EXPECT_FALSE(ancestor.isAncestorOf(unrelated)); // Unrelated paths
}

TEST_F(TMessagePathTest, IsChildOf) {
    TMessagePath parent({entry1_});
    TMessagePath child({entry1_, entry2_});
    TMessagePath unrelated({entry3_});
    
    EXPECT_TRUE(child.isChildOf(parent));
    EXPECT_FALSE(parent.isChildOf(child));
    EXPECT_FALSE(child.isChildOf(unrelated));
}

TEST_F(TMessagePathTest, IsDescendantOf) {
    TMessagePath ancestor({entry1_});
    TMessagePath parent({entry1_, entry2_});
    TMessagePath child({entry1_, entry2_, entry3_});
    TMessagePath unrelated({entry3_});
    
    EXPECT_TRUE(parent.isDescendantOf(ancestor));
    EXPECT_TRUE(child.isDescendantOf(ancestor));
    EXPECT_TRUE(child.isDescendantOf(parent));
    EXPECT_FALSE(ancestor.isDescendantOf(parent));
    EXPECT_FALSE(child.isDescendantOf(unrelated));
}

// Test out of bounds access
TEST_F(TMessagePathTest, OutOfBoundsAccess) {
    TMessagePath path({entry1_});
    EXPECT_THROW(path.at(1), std::out_of_range);
    EXPECT_THROW(path.at(-1), std::out_of_range);
}

} // namespace NOrm::NRelation::Tests
