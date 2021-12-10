#include "mix_bitmap.h"

#include <gtest/gtest.h>

class MixBitmapTest:public ::testing::Test{
protected:
     void SetUp() override;
    void TearDown() override;
public:
    mix_bitmap_t* bitmap;
};

void MixBitmapTest::SetUp(){
    bitmap = mix_bitmap_init(100);
}

void MixBitmapTest::TearDown(){
    mix_bitmap_free(bitmap);
}

TEST_F(MixBitmapTest,BitmapInitTest){
    EXPECT_NE(bitmap,nullptr);
}

TEST_F(MixBitmapTest,BitmapSetTest){
    mix_bitmap_set_bit(10,bitmap);
    mix_bitmap_set_bit(20,bitmap);
    mix_bitmap_set_bit(130,bitmap);
    mix_bitmap_set_bit(119,bitmap);
    EXPECT_EQ(mix_bitmap_test_bit(10,bitmap),1);
    EXPECT_EQ(mix_bitmap_test_bit(20,bitmap),1);
    EXPECT_EQ(mix_bitmap_test_bit(130,bitmap),1);
    EXPECT_EQ(mix_bitmap_test_bit(119,bitmap),1);

    EXPECT_EQ(mix_bitmap_test_bit(11,bitmap),0);
    EXPECT_EQ(mix_bitmap_test_bit(131,bitmap),0);
    EXPECT_EQ(mix_bitmap_test_bit(110,bitmap),0);
    EXPECT_EQ(mix_bitmap_test_bit(111,bitmap),0);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}





