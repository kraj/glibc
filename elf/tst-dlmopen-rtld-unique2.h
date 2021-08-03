static dlmopen_test_spec dltest[] =
  {
   {
    .name = "dlmopen-shared-unique:0:none--ns0",
    .desc = "dlmopen RTLD_SHARED a DF_GNU_1_UNIQUE dso in the base ns",
    .args.dso_path = DSO_UNIQUE,
    .args.ns = LM_ID_BASE,
    .args.flags = RTLD_SHARED,
    .handle_ns = 0,
    .handle_type = DSO,
    .preloaded = { },
    .loaded = { [0] = DSO|NEW },
   },
   {
    .name = "dlmopen-shared-unique:1:ns0--ns0-ns1p",
    .desc = "dlmopen RTLD_SHARED a DF_GNU_1_UNIQUE dso into ns1 while present in ns0",
    .args.dso_path = DSO_UNIQUE,
    .args.ns = LM_ID_NEWLM,
    .args.flags = RTLD_SHARED,
    .handle_ns = EXPECTED_NS,
    .handle_type = PROXY,
    .preloaded = { [0] = DSO },
    .loaded = { [0] = DSO, [EXPECTED_NS] = PROXY|NEW },
   },
  };
