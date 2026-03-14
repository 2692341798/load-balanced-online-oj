#include <gtest/gtest.h>
#include "compile_server/compiler.hpp"
#include "comm/util.hpp"
#include <fstream>

using namespace ns_compiler;
using namespace ns_util;

class CompilerTest : public ::testing::Test {
protected:
    std::string file_name = "test_compile_" + std::to_string(time(nullptr));
    std::string src_path;

    void SetUp() override {
        // Create temp dir if needed
        std::string temp_dir = "./temp";
        if (!FileUtil::IsFileExists(temp_dir)) {
            mkdir(temp_dir.c_str(), 0777);
        }
        
        // Create subdirectory for this test case
        std::string subdir = temp_dir + "/" + file_name;
        mkdir(subdir.c_str(), 0777);
        
        src_path = PathUtil::Src(file_name);
        std::ofstream out(src_path);
        out << "#include <iostream>\nint main() { std::cout << \"Hello Compiler\"; return 0; }";
        out.close();
    }

    void TearDown() override {
        // Cleanup
        unlink(src_path.c_str());
        unlink(PathUtil::Exe(file_name).c_str());
        unlink(PathUtil::CompilerError(file_name).c_str());
    }
};

TEST_F(CompilerTest, CompileCppSuccess) {
    EXPECT_TRUE(Compiler::Compile(file_name));
    EXPECT_TRUE(FileUtil::IsFileExists(PathUtil::Exe(file_name)));
}

TEST_F(CompilerTest, CompileCppError) {
    std::string err_file = "test_compile_err_" + std::to_string(time(nullptr));
    
    // Create subdir for err_file
    std::string err_subdir = "./temp/" + err_file;
    mkdir(err_subdir.c_str(), 0777);
    
    std::string err_path = PathUtil::Src(err_file);
    std::ofstream out(err_path);
    out << "#include <iostream>\nint main() { std::cout << \"Missing Semicolon\" return 0; }";
    out.close();
    
    EXPECT_FALSE(Compiler::Compile(err_file));
    EXPECT_FALSE(FileUtil::IsFileExists(PathUtil::Exe(err_file)));
    EXPECT_TRUE(FileUtil::IsFileExists(PathUtil::CompilerError(err_file)));
    
    // Check error message content
    std::string err_msg;
    FileUtil::ReadFile(PathUtil::CompilerError(err_file), &err_msg, true);
    EXPECT_FALSE(err_msg.empty());
    
    unlink(err_path.c_str());
    unlink(PathUtil::CompilerError(err_file).c_str());
}
