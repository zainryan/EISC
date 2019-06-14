/*****************************************************************************
 *
 *     Author: Xilinx, Inc.
 *
 *     This text contains proprietary, confidential information of
 *     Xilinx, Inc. , is distributed by under license from Xilinx,
 *     Inc., and may be used, copied and/or disclosed only pursuant to
 *     the terms of a valid license agreement with Xilinx, Inc.
 *
 *     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
 *     AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
 *     SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
 *     OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
 *     APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
 *     THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
 *     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
 *     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
 *     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
 *     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
 *     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
 *     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE.
 *
 *     Xilinx products are not intended for use in life support appliances,
 *     devices, or systems. Use in such applications is expressly prohibited.
 *
 *     (c) Copyright 2012-2013 Xilinx Inc.
 *     All rights reserved.
 *
 *****************************************************************************/

/*
 * HLS Video I/O Function Header File
 */

#ifndef ___HLS__VIDEO_IO__
#define ___HLS__VIDEO_IO__

#include "hls_axi_io.h"

namespace hls {

#define ERROR_IO_EOL_EARLY (1 << 0)
#define ERROR_IO_EOL_LATE (1 << 1)
#define ERROR_IO_SOF_EARLY (1 << 0)
#define ERROR_IO_SOF_LATE (1 << 1)

// Unpack a AXI video stream into a hls::Mat<> object

template <int W, int ROWS, int COLS, int T>
int AXIvideo2Mat(stream<ap_axiu<W, 1, 1, 1>> &AXI_video_strm,
                 Mat<ROWS, COLS, T> &img) {
  int res = 0;
  ap_axiu<W, 1, 1, 1> axi;
  Scalar<HLS_MAT_CN(T), HLS_TNAME(T)> pix;
  int depth = HLS_TBITDEPTH(T);
  // std::cout << W << " " << depth << " " << HLS_MAT_CN(T) << "\n";
  assert(W >= depth * HLS_MAT_CN(T) &&
         "Bit-Width of AXI stream must be greater than the total number of "
         "bits in a pixel");
  HLS_SIZE_T rows = img.rows;
  HLS_SIZE_T cols = img.cols;
  assert(rows <= ROWS);
  assert(cols <= COLS);
  bool sof = 0;
loop_wait_for_start:
  while (!sof) {
#pragma HLS pipeline II = 1
#pragma HLS loop_tripcount avg = 0 max = 0
    AXI_video_strm >> axi;
    sof = axi.user.to_int();
  }
loop_height:
  for (HLS_SIZE_T i = 0; i < rows; i++) {
    bool eol = 0;
  loop_width:
    for (HLS_SIZE_T j = 0; j < cols; j++) {
#pragma HLS loop_flatten off
#pragma HLS pipeline II = 1
      if (sof || eol) {
        sof = 0;
        eol = axi.last.to_int();
      } else {
        // If we didn't reach EOL, then read the next pixel
        AXI_video_strm >> axi;
        eol = axi.last.to_int();
        bool user = axi.user.to_int();
        if (user) {
          res |= ERROR_IO_SOF_EARLY;
        }
      }
      if (eol && (j != cols - 1)) {
        res |= ERROR_IO_EOL_EARLY;
      }
    loop_channels:
      for (HLS_CHANNEL_T k = 0; k < HLS_MAT_CN(T); k++) {
        AXIGetBitFields(axi, k * depth, depth, pix.val[k]);
      }
      img << pix;
    }
  loop_wait_for_eol:
    while (!eol) {
#pragma HLS pipeline II = 1
#pragma HLS loop_tripcount avg = 0 max = 0
      // Keep reading until we get to EOL
      AXI_video_strm >> axi;
      eol = axi.last.to_int();
      res |= ERROR_IO_EOL_LATE;
    }
  }
  return res;
}

// Pack the data of a hls::Mat<> object into an AXI Video stream

template <int W, int ROWS, int COLS, int T>
int Mat2AXIvideo(Mat<ROWS, COLS, T> &img,
                 stream<ap_axiu<W, 1, 1, 1>> &AXI_video_strm) {
  int res = 0;
  Scalar<HLS_MAT_CN(T), HLS_TNAME(T)> pix;
  ap_axiu<W, 1, 1, 1> axi;
  int depth = HLS_TBITDEPTH(T);
  // std::cout << W << " " << depth << " " << HLS_MAT_CN(T) << "\n";
  assert(W >= depth * HLS_MAT_CN(T) &&
         "Bit-Width of AXI stream must be greater than the total number of "
         "bits in a pixel");
  HLS_SIZE_T rows = img.rows;
  HLS_SIZE_T cols = img.cols;
  assert(rows <= ROWS);
  assert(cols <= COLS);
  bool sof = 1;
loop_height:
  for (HLS_SIZE_T i = 0; i < rows; i++) {
  loop_width:
    for (HLS_SIZE_T j = 0; j < cols; j++) {
#pragma HLS loop_flatten off
#pragma HLS pipeline II = 1
      if (sof) {
        axi.user = 1;
        sof = 0;
      } else {
        axi.user = 0;
      }
      if (j == (cols - 1)) {
        axi.last = 1;
      } else {
        axi.last = 0;
      }
      img >> pix;
      axi.data = -1;
    loop_channels:
      for (HLS_CHANNEL_T k = 0; k < HLS_MAT_CN(T); k++) {
        AXISetBitFields(axi, k * depth, depth, pix.val[k]);
      }
      axi.keep = -1;
      AXI_video_strm << axi;
    }
  }
  return res;
}

// Read data from AXI-MM, store it to hls::Mat<>

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int Array2Mat(FB_T fb[ROWS * FB_COLS], int rowStride, Mat<ROWS, COLS, T> &img) {
  int res = 0;
  HLS_SIZE_T rows = img.rows;
  HLS_SIZE_T cols = img.cols;
  assert(cols <= rowStride);
  assert(rowStride <= FB_COLS);
  assert(rows <= ROWS);
  assert(cols <= COLS);
  assert(COLS <= FB_COLS);
  int fb_BitWidth = Type_BitWidth<FB_T>::Value;
  int depth = HLS_TBITDEPTH(T);
  int ch = HLS_MAT_CN(T);
  assert(fb_BitWidth >= ch * depth &&
         "Bit-Width of frame buffer must be greater than the total number of "
         "bits in a pixel");
  HLS_SIZE_T col = 0;
  Scalar<HLS_MAT_CN(T), HLS_TNAME(T)> pix;
loop_pixel:
  for (HLS_SIZE_T row = 0; row < rows; row++) {
    for (HLS_SIZE_T col = 0; col < cols; col++) {
#pragma HLS loop_flatten off
#pragma HLS pipeline II = 1
      FB_T fb_pix = fb[row * rowStride + col];
      ap_uint<HLS_MAT_CN(T) * HLS_TBITDEPTH(T)> pix_value;
      AXISetBitFields(pix_value, 0, HLS_MAT_CN(T) * HLS_TBITDEPTH(T), fb_pix);
    loop_channels:
      for (HLS_CHANNEL_T k = 0; k < HLS_MAT_CN(T); k++) {
#pragma HLS unroll
        AXIGetBitFields(pix_value, k * depth, depth, pix.val[k]);
      }
      img << pix;
    }
  }
  return res;
}

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int Array2Mat(FB_T fb[ROWS * FB_COLS], Mat<ROWS, COLS, T> &img) {
  int res = 0;
  res = Array2Mat<FB_COLS>(fb, FB_COLS, img);
  return res;
}

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int AXIM2Mat(FB_T fb[ROWS * FB_COLS], Mat<ROWS, COLS, T> &img) {
  int res = 0;
  res = Array2Mat<FB_COLS>(fb, FB_COLS, img);
  return res;
}

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int AXIM2Mat(FB_T fb[ROWS * FB_COLS], int rowStride, Mat<ROWS, COLS, T> &img) {
  int res = 0;
  res = Array2Mat<FB_COLS>(fb, rowStride, img);
  return res;
}

// Read data from hls::Mat<>, store it to AXI-MM

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int Mat2Array(Mat<ROWS, COLS, T> &img, FB_T fb[ROWS * FB_COLS], int rowStride) {
  int res = 0;
  HLS_SIZE_T rows = img.rows;
  HLS_SIZE_T cols = img.cols;
  assert(cols <= rowStride);
  assert(rowStride <= FB_COLS);
  assert(rows <= ROWS);
  assert(cols <= COLS);
  assert(COLS <= FB_COLS);
  int fb_BitWidth = Type_BitWidth<FB_T>::Value;
  int depth = HLS_TBITDEPTH(T);
  int ch = HLS_MAT_CN(T);
  assert(fb_BitWidth >= ch * depth &&
         "Bit-Width of frame buffer must be greater than the total number of "
         "bits in a pixel");
  HLS_SIZE_T col = 0;
  Scalar<HLS_MAT_CN(T), HLS_TNAME(T)> pix;
loop_pixel:
  for (HLS_SIZE_T row = 0; row < rows; row++) {
    for (HLS_SIZE_T col = 0; col < cols; col++) {
#pragma HLS loop_flatten off
#pragma HLS pipeline II = 1
      ap_uint<HLS_MAT_CN(T) * HLS_TBITDEPTH(T)> pix_value;
      img >> pix;
    loop_channels:
      for (HLS_CHANNEL_T k = 0; k < HLS_MAT_CN(T); k++) {
#pragma HLS unroll
        AXISetBitFields(pix_value, k * depth, depth, pix.val[k]);
      }
      FB_T fb_pix;
      AXIGetBitFields(pix_value, 0, HLS_MAT_CN(T) * HLS_TBITDEPTH(T), fb_pix);
      fb[row * rowStride + col] = fb_pix;
    }
  }
  return res;
}

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int Mat2Array(Mat<ROWS, COLS, T> &img, FB_T fb[ROWS * FB_COLS]) {
  int res = 0;
  res = Mat2Array<FB_COLS>(img, fb, FB_COLS);
  return res;
}

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int Mat2AXIM(Mat<ROWS, COLS, T> &img, FB_T fb[ROWS * FB_COLS]) {
  int res = 0;
  res = Mat2Array<FB_COLS>(img, fb, FB_COLS);
  return res;
}

template <int FB_COLS, typename FB_T, int ROWS, int COLS, int T>
int Mat2AXIM(Mat<ROWS, COLS, T> &img, FB_T fb[ROWS * FB_COLS], int rowStride) {
  int res = 0;
  res = Mat2Array<FB_COLS>(img, fb, rowStride);
  return res;
}

} // namespace hls

#endif

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
