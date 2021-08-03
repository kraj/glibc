static dlmopen_test_spec dltest[] =
  {
   {
    .name = "dlmopen-preload:X:none--nsX",
    .desc = "preload a DSO into ns1 to prepare for other tests",
    .is_prep_stage = 1,
    .args.dso_path = DSO_NORMAL,
    .args.ns = LM_ID_NEWLM,
    .handle_ns = EXPECTED_NS,
    .handle_type = DSO,
    .preloaded = { },
    .loaded = { [EXPECTED_NS] = DSO|NEW },
   },
   {
    .name = "dlmopen-shared:0:nsX--nsX-ns0",
    .desc = "dlmopen RTLD_SHARED into ns0 when preloaded into nsX",
    .args.dso_path = DSO_NORMAL,
    .args.ns = LM_ID_BASE,
    .args.flags = RTLD_SHARED,
    .handle_ns = 0,
    .handle_type = DSO,
    .preloaded = { [EXPECTED_NS] = DSO },
    .loaded = { [0] = DSO|NEW, [EXPECTED_NS] = DSO },
   },
  };
