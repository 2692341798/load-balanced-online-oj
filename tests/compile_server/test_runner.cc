#include <gtest/gtest.h>
#include "compile_server/runner.hpp"
#include "compile_server/compiler.hpp"
#include "comm/util.hpp"
#include <fstream>

using namespace ns_runner;
using namespace ns_compiler;
using namespace ns_util;

class RunnerTest : public ::testing::Test {
protected:
    std::string file_name = "test_run_" + std::to_string(time(nullptr));
    std::string src_path;

    void SetUp() override {
        // Create temp dir
        std::string temp_dir = "./temp";
        if (!FileUtil::IsFileExists(temp_dir)) mkdir(temp_dir.c_str(), 0777);
        
        // Create subdirectory
        std::string subdir = temp_dir + "/" + file_name;
        mkdir(subdir.c_str(), 0777);
        
        src_path = PathUtil::Src(file_name);
    }

    void TearDown() override {
        // Cleanup
        unlink(src_path.c_str());
        unlink(PathUtil::Exe(file_name).c_str());
        unlink(PathUtil::Stdin(file_name).c_str());
        unlink(PathUtil::Stdout(file_name).c_str());
        unlink(PathUtil::Stderr(file_name).c_str());
        unlink(PathUtil::CompilerError(file_name).c_str());
    }
};

TEST_F(RunnerTest, RunCppSuccess) {
    std::ofstream out(src_path);
    out << "#include <iostream>\nint main() { std::cout << \"Hello Runner\"; return 0; }";
    out.close();

    ASSERT_TRUE(Compiler::Compile(file_name));
    
    // Set limit: 1s CPU, 30MB Mem (30 * 1024 KB)
    EXPECT_EQ(Runner::Run(file_name, 1, 30 * 1024), 0);
    
    std::string output;
    FileUtil::ReadFile(PathUtil::Stdout(file_name), &output, true);
    EXPECT_EQ(output, "Hello Runner\n");
}

TEST_F(RunnerTest, RunCppTimeLimit) {
    std::ofstream out(src_path);
    // Infinite loop
    out << "#include <iostream>\nint main() { while(1); return 0; }";
    out.close();

    ASSERT_TRUE(Compiler::Compile(file_name));
    
    // Set limit: 1s CPU
    int status = Runner::Run(file_name, 1, 30 * 1024);
    EXPECT_GT(status, 0); // Should be killed by signal
    // LOG(INFO) << "Time Limit Exceeded Status: " << status << "\n";
}

TEST_F(RunnerTest, RunCppMemoryLimit) {
    std::ofstream out(src_path);
    // Allocate lots of memory
    out << "#include <iostream>\n#include <vector>\nint main() { while(1) { int* p = new int[1024*1024]; } return 0; }";
    out.close();

    ASSERT_TRUE(Compiler::Compile(file_name));
    
    // Set limit: 1s CPU, 10MB Mem
    int status = Runner::Run(file_name, 1, 10 * 1024);
    EXPECT_GT(status, 0); // Should be killed
}
