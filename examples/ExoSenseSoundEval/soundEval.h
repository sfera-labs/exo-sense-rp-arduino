#include "kiss_fftr.h"

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

#define SNDEV_PERIOD_SAMPLES_SLOW    (SNDEV_PERIOD_MS_SLOW * SNDEV_SAMPLING_FREQ_HZ / 1000) // = 40000
#define SNDEV_PERIOD_SAMPLES_FAST    (SNDEV_PERIOD_MS_FAST * SNDEV_SAMPLING_FREQ_HZ / 1000) // = 5000
#define SNDEV_PERIOD_SAMPLES_IMPULSE (SNDEV_PERIOD_MS_IMPULSE * SNDEV_SAMPLING_FREQ_HZ / 1000) // = 1400

#define SNDEV_SAMPLES_BUFF_SIZE   5000
#define SNDEV_FFT_OUTPUT_SIZE     (SNDEV_SAMPLES_BUFF_SIZE / 2 + 1)

#define SNDEV_P_ZERO 0.00002

#define SNDEV_FREQ_BANDS 36

static int _freqW = SNDEV_FREQ_WEIGHTING_A;
static int _periodSampleSize;

static float _samplesBuff[SNDEV_SAMPLES_BUFF_SIZE];
static int _samplesBuffIdx;
static int _samplesBuffMax;
static float _samplesTimeSec;
static float _windowFun[SNDEV_SAMPLES_BUFF_SIZE];
static float _periodPower;
static float _periodRounds;
static int _periodPowerCnt;
static float _freqBandPow[SNDEV_FREQ_BANDS];
static float _refPZ2 = pow(SNDEV_P_ZERO, 2);
static float _refOnePa;
static kiss_fft_cpx _fftOut[SNDEV_FFT_OUTPUT_SIZE];
static kiss_fftr_cfg _fftCfg;

static float _lEqBand[SNDEV_FREQ_BANDS];
static float _lEqPeriodDb;

static float _weightingTable[SNDEV_FREQ_BANDS][4] = {
   // freq,    A,        C,       Z
    { 6.3,     -85.4,    -21.3,    0 },
    { 8,       -77.8,    -17.7,    0 },
    { 10,      -70.4,    -14.3,    0 },
    { 12.5,    -63.4,    -11.2,    0 },
    { 16,      -56.7,    -8.5,     0 },
    { 20,      -50.5,    -6.2,     0 },
    { 25,      -44.7,    -4.4,     0 },
    { 31.5,    -39.4,    -3,       0 },
    { 40,      -34.6,    -2,       0 },
    { 50,      -30.2,    -1.3,     0 },
    { 63,      -26.2,    -0.8,     0 },
    { 80,      -22.5,    -0.5,     0 },
    { 100,     -19.1,    -0.3,     0 },
    { 125,     -16.1,    -0.2,     0 },
    { 160,     -13.4,    -0.1,     0 },
    { 200,     -10.9,    0,        0 },
    { 250,     -8.6,     0,        0 },
    { 315,     -6.6,     0,        0 },
    { 400,     -4.8,     0,        0 },
    { 500,     -3.2,     0,        0 },
    { 630,     -1.9,     0,        0 },
    { 800,     -0.8,     0,        0 },
    { 1000,    0.0,      0,        0 },
    { 1250,    0.6,      0,        0 },
    { 1600,    1.0,      -0.1,     0 },
    { 2000,    1.2,      -0.2,     0 },
    { 2500,    1.3,      -0.3,     0 },
    { 3150,    1.2,      -0.5,     0 },
    { 4000,    1.0,      -0.8,     0 },
    { 5000,    0.5,      -1.3,     0 },
    { 6300,    -0.1,     -2,       0 },
    { 8000,    -1.1,     -3,       0 },
    { 10000,   -2.5,     -4.4,     0 },
    { 12500,   -4.3,     -6.2,     0 },
    { 16000,   -6.6,     -8.5,     0 },
    { 20000,   -9.3,     -11.2,    0 },
};

bool soundEvalSetMicSpecs(float sensitivityDb, int32_t sampleMaxVal) {
  _refOnePa = sampleMaxVal * pow(10, sensitivityDb / 20.0);
  return true;
}

bool soundEvalSetTimeWeighting(int tw) {
  float periodTimeMs;
  switch (tw) {
    case SNDEV_TIME_WEIGHTING_FAST:
      _periodSampleSize = SNDEV_PERIOD_SAMPLES_FAST;
      periodTimeMs = SNDEV_PERIOD_MS_FAST;
      break;
    case SNDEV_TIME_WEIGHTING_IMPULSE:
      _periodSampleSize = SNDEV_PERIOD_SAMPLES_IMPULSE;
      periodTimeMs = SNDEV_PERIOD_MS_IMPULSE;
      break;
    case SNDEV_TIME_WEIGHTING_SLOW:
      _periodSampleSize = SNDEV_PERIOD_SAMPLES_SLOW;
      periodTimeMs = SNDEV_PERIOD_MS_SLOW;
      break;
    default:
      return false;
  }
  _samplesBuffMax = min(_periodSampleSize, SNDEV_SAMPLES_BUFF_SIZE);
  _periodRounds = _periodSampleSize / _samplesBuffMax;
  _samplesTimeSec = periodTimeMs / _periodRounds / 1000.0;
  _samplesBuffIdx = 0;
  _periodPower = 0;
  _periodPowerCnt = 0;
  for (int i = 0; i < _samplesBuffMax; i++) {
    _windowFun[i] = (1.0 - cos(2.0 * M_PI * i / _samplesBuffMax)) * 0.5;
  }
  _fftCfg = kiss_fftr_alloc(_samplesBuffMax, false, NULL, NULL);
  if (!_fftCfg) {
    return false;
  }
  return true;
}

bool soundEvalSetFreqWeighting(int fw) {
  switch (fw) {
    case SNDEV_FREQ_WEIGHTING_A:
    case SNDEV_FREQ_WEIGHTING_C:
    case SNDEV_FREQ_WEIGHTING_Z:
      _freqW = fw;
      return true;
    default:
      return false;
  }
}

static float getSamplesWeightedPower(float samplesPower, int samplesSize) {
  float totPwr = 0;
  float totWeightedPwr = 0;
  int freqBandIdx = 0;
  
  for (int i = 0; i < SNDEV_FREQ_BANDS; i++) {
    _freqBandPow[i] = 0;
  }

  kiss_fftr(_fftCfg , _samplesBuff, _fftOut);

  for (int i = 0; i < samplesSize / 2 + 1; i++) {
    float freq = i / _samplesTimeSec;
    float freqPwr = (_fftOut[i].r * _fftOut[i].r) + (_fftOut[i].i * _fftOut[i].i);
    
    if (freq <= SNDEV_SAMPLING_FREQ_HZ / 2) {
      if (freq > _weightingTable[freqBandIdx][SNDEV_FREQ_WEIGHTING_FREQ]) {
        freqBandIdx++;
      }
      _freqBandPow[freqBandIdx] += freqPwr;
      totPwr += freqPwr;
    }
  }

  for (int i = 0; i < SNDEV_FREQ_BANDS; i++) {
    float freqBandWeight = _freqBandPow[i] / totPwr;
    float freqBandWeightedPow = freqBandWeight * (samplesPower / samplesSize);
    _lEqBand[i] = 10 * log10(freqBandWeightedPow) + _weightingTable[i][_freqW];
    totWeightedPwr += pow(10, _lEqBand[i] / 10);
  }

  return totWeightedPwr;
}

static void processSamples() {  
  float samplesPower = 0;
  for (int i = 0; i < _samplesBuffMax; i++) {
    float pressure = _samplesBuff[i] / _refOnePa;
    samplesPower += pressure * pressure / _refPZ2;
    /* Apply Hann window function to decrease spectral leakage */
    _samplesBuff[i] = pressure * _windowFun[i];
  }

  _periodPowerCnt += _samplesBuffMax;
  _periodPower += getSamplesWeightedPower(samplesPower, _samplesBuffMax);
  
  if (_periodPowerCnt >= _periodSampleSize) {
    _lEqPeriodDb = 10 * log10(_periodPower / _periodRounds);
    _periodPowerCnt = 0;
    _periodPower = 0;

    Serial.println(" === _lEqPeriodDb ==="); // TODO remove
    Serial.println(_lEqPeriodDb); // TODO remove
    Serial.println(" ===================="); // TODO remove
  }
}

void soundEvalProcess(int32_t sample) {
  _samplesBuff[_samplesBuffIdx] = sample;
  if (++_samplesBuffIdx >= _samplesBuffMax) {
    processSamples();
    _samplesBuffIdx = 0;
  }
}
