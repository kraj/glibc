static dlmopen_test_spec dltest[] =
  {
   {
    .name = "dlmopen-shared-unique:X:none--ns0-nsXp",
    .desc = "dlmopen a DF_GNU_1_UNIQUE dso",
    .args.dso_path = DSO_UNIQUE,
    .args.ns = LM_ID_NEWLM,
    .args.flags = RTLD_SHARED,
    .handle_ns = EXPECTED_NS,
    .handle_type = PROXY,
    .preloaded = { },
    .loaded = { [0] = DSO|NEW, [EXPECTED_NS] = PROXY|NEW },
   },
  };
