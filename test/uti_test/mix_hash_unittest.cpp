#include <gtest/gtest.h>

#include <iostream>
#include <thread>
#include <memory>
#include <unistd.h>

#include "mix_hash.h"


class MixHashTest:public ::testing::Test{
protected:
    void SetUp() override;
    void TearDown() override;
    
    
    mix_hash_t* hash;
    const static int hash_size = 10;
    const static int count = 100000;
    const static int thread_num = 10;
};

void MixHashTest::SetUp(){
    hash = mix_hash_init(MixHashTest::hash_size);
}

void MixHashTest::TearDown(){
    //std::cout<<"call tear down"<<std::endl;
    //mix_hash_free(hash);
}

TEST_F(MixHashTest,InitTest){
    EXPECT_NE(hash,nullptr);
}

TEST_F(MixHashTest,SingleThreadTest){
    //GTEST_SKIP();
    for(int i = 0; i < MixHashTest::count;i++){
        mix_hash_put(hash,i,i);
    }

    for(int i = 0; i < MixHashTest::count;i++){
        EXPECT_EQ(mix_hash_get(hash,i),i);
    }
}

TEST_F(MixHashTest,MultiThreadsTest){
    //GTEST_SKIP();
    std::thread threads[MixHashTest::thread_num];

    for(int i = 0; i < MixHashTest::thread_num; i++){
        int start = i;
        threads[i] = std::thread([=](){
            for(int j = start; j < MixHashTest::count; j+= MixHashTest::thread_num){
                mix_hash_put(hash,j,j);
            } 
        });
    }

    for(int i = 0; i < MixHashTest::thread_num; i++){
        threads[i].join();
    }

    for(int i = 0; i < MixHashTest::count; i++){
        EXPECT_EQ(mix_hash_get(hash,i),i);
    }
}

TEST_F(MixHashTest,GetTest){
    
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}