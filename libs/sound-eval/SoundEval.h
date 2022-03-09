/*
  SoundEval.h - Sound Evaluation library

    Copyright (C) 2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef SoundEval_h
#define SoundEval_h

#define SNDEV_SAMPLING_FREQ_HZ   40000

#define SNDEV_TIME_WEIGHTING_SLOW    1
#define SNDEV_TIME_WEIGHTING_FAST    2
#define SNDEV_TIME_WEIGHTING_IMPULSE 3

#define SNDEV_FREQ_WEIGHTING_FREQ   0
#define SNDEV_FREQ_WEIGHTING_A      1
#define SNDEV_FREQ_WEIGHTING_C      2
#define SNDEV_FREQ_WEIGHTING_Z      3

#define SNDEV_PERIOD_MS_SLOW      1000
#define SNDEV_PERIOD_MS_FAST      125
#define SNDEV_PERIOD_MS_IMPULSE   35

class SoundEvalClass {
  public:
    typedef void SoundEvalCallback(float lEqDb);
    SoundEvalClass();
    bool setMicSpecs(float sensitivityDb, int32_t sampleMaxVal);
    bool setFreqWeighting(int fw);
    bool setTimeWeighting(int tw);
    void setPeriodResultCallback(SoundEvalCallback* cb);
    void process(int32_t sample);
};

extern SoundEvalClass SoundEval;

#endif
