
extern double log_vlen2 (double);
extern double exp_vlen2 (double);
extern double pow_vlen2 (double, double);

extern double log_vlen4 (double);
extern double exp_vlen4 (double);
extern double pow_vlen4 (double, double);

extern double log_vlen4_avx2 (double);
extern double exp_vlen4_avx2 (double);
extern double pow_vlen4_avx2 (double, double);

extern float logf_vlen4 (float);
extern float expf_vlen4 (float);
extern float powf_vlen4 (float, float);

extern float logf_vlen8 (float);
extern float expf_vlen8 (float);
extern float powf_vlen8 (float, float);

extern float logf_vlen8_avx2 (float);
extern float expf_vlen8_avx2 (float);
extern float powf_vlen8_avx2 (float, float);

int main( void)
{
  double res = 0.0;
  float resf = 0.0;

  res += log_vlen2(1.0);
  res += exp_vlen2(1.0);
  res += pow_vlen2(1.1, 1.1);

  res += log_vlen4(1.0);
  res += exp_vlen4(1.0);
  res += pow_vlen4(1.1, 1.1);

  res += log_vlen4_avx2(1.0);
  res += exp_vlen4_avx2(1.0);
  res += pow_vlen4_avx2(1.1, 1.1);

  resf += logf_vlen4(1.0);
  resf += expf_vlen4(1.0);
  resf += powf_vlen4(1.1, 1.1);

  resf += logf_vlen8(1.0);
  resf += expf_vlen8(1.0);
  resf += powf_vlen8(1.1, 1.1);

  resf += logf_vlen8_avx2(1.0);
  resf += expf_vlen8_avx2(1.0);
  resf += powf_vlen8_avx2(1.1, 1.1);

  return 0;
}
