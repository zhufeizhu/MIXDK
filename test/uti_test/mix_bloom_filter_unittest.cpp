#include "mix_bloom_filter.h"

#include <gtest/gtest.h>

class MixBloomFilterTest: public ::testing::Test{
protected:
    void SetUp() override;
    void TearDown() override;

public:
    mix_counting_bloom_filter_t* bloom_filter;
};

void MixBloomFilterTest::SetUp(){
    bloom_filter = mix_new_counting_bloom_filter(10000,0.01);
}

void MixBloomFilterTest::TearDown(){
    mix_free_counting_bloom_filter(bloom_filter);
}

TEST_F(MixBloomFilterTest,BloomFilterInitTest){
    EXPECT_NE(bloom_filter,nullptr);
}

TEST_F(MixBloomFilterTest,BloomFilterIncrementTest){
    mix_counting_bloom_filter_add(bloom_filter,1000);

    EXPECT_EQ(mix_counting_bloom_filter_test(bloom_filter,1000),1);
    EXPECT_EQ(mix_counting_bloom_filter_test(bloom_filter,1003),0);
    EXPECT_EQ(mix_counting_bloom_filter_test(bloom_filter,12001),0);

    EXPECT_EQ(mix_counting_bloom_filter_test(bloom_filter,56001),0);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}