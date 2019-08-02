#include <gtest/gtest.h>

#include <string>

#include "httpfake.h"
#include "primary/aktualizr.h"
#include "test_utils.h"
#include "uptane_test_common.h"

boost::filesystem::path aktualizr_repo_path;

/*
 * Detect a mismatch in the Targets metadata from Director and Images repo.
 */
TEST(Aktualizr, HardwareMismatch) {
  TemporaryDirectory temp_dir;
  TemporaryDirectory meta_dir;
  auto http = std::make_shared<HttpFake>(temp_dir.Path(), "", meta_dir.Path() / "repo");
  Config conf = UptaneTestCommon::makeTestConfig(temp_dir, http->tls_server);
  logger_set_threshold(boost::log::trivial::trace);

  Process akt_repo(aktualizr_repo_path.string());
  akt_repo.run({"generate", "--path", meta_dir.PathString(), "--correlationid", "abc123"});
  // Note bad hardware ID in the image repo metadata. If we were to use an
  // invalid value in the Director, it gets rejected even earlier.
  akt_repo.run({"image", "--path", meta_dir.PathString(), "--filename", "tests/test_data/firmware.txt", "--targetname",
                "firmware.txt", "--hwid", "hw-mismatch", "--url", "test-url"});
  akt_repo.run({"addtarget", "--path", meta_dir.PathString(), "--targetname", "firmware.txt", "--hwid", "primary_hw",
                "--serial", "CA:FE:A6:D2:84:9D"});
  akt_repo.run({"signtargets", "--path", meta_dir.PathString()});
  // Work around inconsistent directory naming.
  Utils::copyDir(meta_dir.Path() / "repo/image", meta_dir.Path() / "repo/repo");

  auto storage = INvStorage::newStorage(conf.storage);
  UptaneTestCommon::TestAktualizr aktualizr(conf, storage, http);
  aktualizr.Initialize();

  result::UpdateCheck update_result = aktualizr.CheckUpdates().get();
  EXPECT_EQ(update_result.status, result::UpdateStatus::kError);
  EXPECT_EQ(update_result.message, "Target mismatch.");
}

#ifndef __NO_MAIN__
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (argc != 2) {
    std::cerr << "Error: " << argv[0] << " requires the path to the aktualizr-repo utility\n";
    return EXIT_FAILURE;
  }
  aktualizr_repo_path = argv[1];

  logger_init();
  logger_set_threshold(boost::log::trivial::trace);

  return RUN_ALL_TESTS();
}
#endif

// vim: set tabstop=2 shiftwidth=2 expandtab: