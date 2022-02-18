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

#define SNDEV_PERIOD_SAMPLES_SLOW    (SNDEV_PERIOD_MS_SLOW * SNDEV_SAMPLING_FREQ_HZ / 1000)
#define SNDEV_PERIOD_SAMPLES_FAST    (SNDEV_PERIOD_MS_FAST * SNDEV_SAMPLING_FREQ_HZ / 1000)
#define SNDEV_PERIOD_SAMPLES_IMPULSE (SNDEV_PERIOD_MS_IMPULSE * SNDEV_SAMPLING_FREQ_HZ / 1000)

#define SNDEV_SAMPLES_BUFF_SIZE (SNDEV_PERIOD_SAMPLES_SLOW / 8)

#define SNDEV_SAMPLE_DATA_FMT_S24LE 1
#define SNDEV_FMT_S24LE_VAL_MAX 0x7FFFFF

#define SNDEV_P_ZERO 0.00002d

#define SNDEV_FREQ_BANDS 36

static int _periodTimeMs = SNDEV_PERIOD_MS_SLOW;
static int _periodSampleSize = SNDEV_PERIOD_SAMPLES_SLOW;
static int _freqW = SNDEV_FREQ_WEIGHTING_A;

static double _samplesBuff[SNDEV_SAMPLES_BUFF_SIZE];
static int _samplesBuffIdx;
static int _samplesBuffMax;
static double _windowFun[SNDEV_SAMPLES_BUFF_SIZE];
static double _periodPower;
static int _periodPowerCnt;
static double _fftOut[SNDEV_SAMPLES_BUFF_SIZE / 2 + 1][2];
static double _refPZ2 = pow(SNDEV_P_ZERO, 2);
static double _refOnePa;

static double _lEqBand[SNDEV_FREQ_BANDS];
static double _lEqPeriodDb;

static double _weightingTable[SNDEV_FREQ_BANDS][4] = {
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

bool soundEvalSetMicSpecs(double sensitivityDb, int sampleBits, int dataFormat) {
  if (sampleBits != 32) {
    return false;
  }
  if (dataFormat != SNDEV_SAMPLE_DATA_FMT_S24LE) {
    return false;
  }
  _refOnePa = SNDEV_FMT_S24LE_VAL_MAX * pow(10, sensitivityDb / 20.0);
  return true;
}

void soundEvalSetTimeWeighting(int tw) {
  switch (tw) {
    case SNDEV_TIME_WEIGHTING_FAST:
      _periodSampleSize = SNDEV_PERIOD_SAMPLES_FAST;
      _periodTimeMs = SNDEV_PERIOD_MS_FAST;
      break;
    case SNDEV_TIME_WEIGHTING_IMPULSE:
      _periodSampleSize = SNDEV_PERIOD_SAMPLES_IMPULSE;
      _periodTimeMs = SNDEV_PERIOD_MS_IMPULSE;
      break;
    default:
      _periodSampleSize = SNDEV_PERIOD_SAMPLES_SLOW;
      _periodTimeMs = SNDEV_PERIOD_MS_SLOW;
  }
  _samplesBuffIdx = 0;
  _samplesBuffMax = min(_periodSampleSize, SNDEV_SAMPLES_BUFF_SIZE);
  _periodPower = 0;
  _periodPowerCnt = 0;
  for (int i = 0; i < _samplesBuffMax; i++) {
    _windowFun[i] = (1.0 - cos(2.0 * M_PI * i / _samplesBuffMax)) * 0.5;
  }
}

void soundEvalSetFreqWeighting(int fw) {
  _freqW = fw;
}

static void fftExec(double * in, int size, double out[][2]) {
  // TODO
  for (int i = 0; i < size / 2 + 1; i++) {
    out[i][0] = 1;
    out[i][1] = 1;
  }
}

static double getSamplesWeightedPower(double samplesPower, int samplesSize) {
  Serial.println("getSamplesWeightedPower()"); // TODO remove
  Serial.println(samplesPower / 1000000); // TODO remove
  Serial.println(samplesSize); // TODO remove
  
  double totPwr = 0;
  double totWeightedPwr = 0;
  int freqBandIdx = 0;
  double freqBandPow[SNDEV_FREQ_BANDS];
  
  for (int i = 0; i < SNDEV_FREQ_BANDS; i++) {
    freqBandPow[i] = 0;
  }

  fftExec(_samplesBuff, samplesSize, _fftOut);

  for (int i = 0; i < samplesSize / 2 + 1; i++) {
    double freq = i / (_periodTimeMs / 1000.0);
    double freqPwr = pow(_fftOut[i][0], 2) + pow(_fftOut[i][1], 2);
    
    if (freq <= SNDEV_SAMPLING_FREQ_HZ / 2) {
      if (freq > _weightingTable[freqBandIdx][SNDEV_FREQ_WEIGHTING_FREQ]) {
        freqBandIdx++;
      }
      freqBandPow[freqBandIdx] += freqPwr;
      totPwr += freqPwr;
    }
  }

  if (totPwr == 0) {
    // TODO
  }

  for (int i = 0; i < SNDEV_FREQ_BANDS; i++) {
    double freqBandWeight = freqBandPow[i] / totPwr;
    double freqBandWeightedPow = freqBandWeight * (samplesPower / samplesSize);
    _lEqBand[i] = 10 * log10(freqBandWeightedPow) + _weightingTable[i][_freqW];
    totWeightedPwr += pow(10, _lEqBand[i] / 10);
  }

  return totWeightedPwr;
}

static void processSamples() {
  Serial.println("processSamples()"); // TODO remove
  Serial.println("_samplesBuffMax"); // TODO remove
  Serial.println(_samplesBuffMax); // TODO remove
  
  double samplesPower = 0;
  for (int i = 0; i < _samplesBuffMax; i++) {
    double pressure = _samplesBuff[i] / _refOnePa;
    samplesPower += pow(pressure, 2) / _refPZ2;
    /* Apply Hann window function to decrease spectral leakage */
    _samplesBuff[i] = pressure * _windowFun[i];
  }
  
  _periodPowerCnt += _samplesBuffMax;
  _periodPower += getSamplesWeightedPower(samplesPower, _samplesBuffMax);

  Serial.println("_periodPowerCnt"); // TODO remove
  Serial.println(_periodPowerCnt); // TODO remove

  Serial.println("---"); // TODO remove
  
  if (_periodPowerCnt >= _periodSampleSize) {
    _lEqPeriodDb = 10 * log10(_periodPower / _periodSampleSize);
    _periodPowerCnt = 0;
    _periodPower = 0;

    Serial.println("_lEqPeriodDb"); // TODO remove
    Serial.println(_lEqPeriodDb); // TODO remove

    while(1); // TODO remove
  }
}

void soundEvalProcess(unsigned int size, uint8_t* buff) {
  Serial.println("soundEvalProcess()"); // TODO remove
  Serial.println(size); // TODO remove
  
  for (int i = 0; i < size; i += 4) {
    /* convert 32 bits samples with 24 bits (signed, little-endian) data */
    int32_t val = (unsigned int) buff[i + 2] * 65536
                + (unsigned int) buff[i + 1] * 256
                + (unsigned int) buff[i];
    if ((val & 0x800000) == 0x800000) {
      val |= 0xff000000;
    }

    // val = val >> 10; // TODO remove
    
    _samplesBuff[_samplesBuffIdx] = val;

    if (++_samplesBuffIdx >= _samplesBuffMax) {
      processSamples();
      _samplesBuffIdx = 0;
    }
  }
}
