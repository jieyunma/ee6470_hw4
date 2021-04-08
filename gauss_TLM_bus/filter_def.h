#ifndef FILTER_DEF_H_
#define FILTER_DEF_H_

// sobel mask parameters
const int MASK_N = 1;
const int MASK_X = 3;
const int MASK_Y = 3;

// Sobel Filter inner transport addresses
// Used between blocking_transport() & do_filter()
const int GAUSS_FILTER_R_ADDR = 0x00000000;
const int GAUSS_FILTER_RESULT_ADDR = 0x00000004;

union word {
  int sint;
  unsigned int uint;
  unsigned char uc[4];
};

const int mask[MASK_X][MASK_Y] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

#endif
