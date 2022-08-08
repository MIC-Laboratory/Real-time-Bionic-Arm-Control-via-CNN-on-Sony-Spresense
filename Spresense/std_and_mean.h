/*
  Description:
    Mean and Standard Deviation for each Myo channel, calculated from 7 gestues' sEMG in NinaPro DB5
  Date: Summer 2022
*/ 

const float s_params[8][8] = {
    {-0.90727645, 14.052758}, // [0][0] is MEAN, [0][1] is STD of Myo Channel 1
    {-1.0710329, 16.929909},  // [1][0] is MEAN, [1][1] is STD of Myo Channel 2
    {-0.9071628, 7.653704},   // [2][0] is MEAN, [2][1] is STD of Myo Channel 3
    {-0.83182526, 4.6818013}, // [3][0] is MEAN, [3][1] is STD of Myo Channel 4
    {-0.89715457, 11.104531}, // [4][0] is MEAN, [4][1] is STD of Myo Channel 5
    {-0.8876045, 12.34298},   // [5][0] is MEAN, [5][1] is STD of Myo Channel 6
    {-0.87025106, 10.944115}, // [6][0] is MEAN, [6][1] is STD of Myo Channel 7
    {-0.903, 12.639286}       // [7][0] is MEAN, [7][1] is STD of Myo Channel 8
};