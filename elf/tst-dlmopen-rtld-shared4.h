static dlmopen_test_spec dltest[] =
  {
   {
    .name = "dlmopen-shared:X:none--ns0-nsX",
    .desc = "dlmopen a new proxy in nsX with no preexisting dso in ns0",
    .args.dso_path = DSO_NORMAL,
    .args.ns = LM_ID_NEWLM,
    .args.flags = RTLD_SHARED,
    .handle_ns = EXPECTED_NS,
    .handle_type = PROXY,
    .preloaded = { },
    .loaded = { [0] = DSO|NEW, [EXPECTED_NS] = PROXY|NEW },
   },
  };
