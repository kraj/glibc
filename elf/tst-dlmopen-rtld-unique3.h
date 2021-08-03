static dlmopen_test_spec dltest[] =
  {
   {
    .name = "dlmopen-unique:X:none--ns0-ns1p",
    .desc = "dlmopen a DF_GNU_1_UNIQUE dso into nsX",
    .args.dso_path = DSO_UNIQUE,
    .args.ns = LM_ID_NEWLM,
    .handle_ns = EXPECTED_NS,
    .handle_type = PROXY,
    .preloaded = { },
    .loaded = { [0] = DSO|NEW, [EXPECTED_NS] = PROXY|NEW },
   },
  };
