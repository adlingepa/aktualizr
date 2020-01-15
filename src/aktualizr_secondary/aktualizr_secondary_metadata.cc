#include "aktualizr_secondary_metadata.h"

Metadata::Metadata(const Uptane::RawMetaPack& meta_pack)
    : _director_metadata{{Uptane::Role::ROOT, meta_pack.director_root},
                         {Uptane::Role::TARGETS, meta_pack.director_targets}},
      _image_metadata{
          {Uptane::Role::ROOT, meta_pack.image_root},
          {Uptane::Role::TIMESTAMP, meta_pack.image_timestamp},
          {Uptane::Role::SNAPSHOT, meta_pack.image_snapshot},
          {Uptane::Role::TARGETS, meta_pack.image_targets},
      } {
  director_root_version = Uptane::Version(Uptane::extractVersionUntrusted(meta_pack.director_root));
  image_root_version = Uptane::Version(Uptane::extractVersionUntrusted(meta_pack.image_root));
}

bool Metadata::fetchRole(std::string* result, int64_t maxsize, Uptane::RepositoryType repo, const Uptane::Role& role,
                         Uptane::Version version) const {
  (void)maxsize;

  return getRoleMetadata(result, repo, role, version);
}

bool Metadata::fetchLatestRole(std::string* result, int64_t maxsize, Uptane::RepositoryType repo,
                               const Uptane::Role& role) const {
  (void)maxsize;
  return getRoleMetadata(result, repo, role, Uptane::Version());
}

bool Metadata::getRoleMetadata(std::string* result, const Uptane::RepositoryType& repo, const Uptane::Role& role,
                               Uptane::Version version) const {
  const std::unordered_map<std::string, std::string>* metadata_map = nullptr;

  if (repo == Uptane::RepositoryType::Director()) {
    metadata_map = &_director_metadata;
  } else if (repo == Uptane::RepositoryType::Image()) {
    metadata_map = &_image_metadata;
  }

  if (metadata_map == nullptr) {
    LOG_ERROR << "There are no any metadata for the given type of repository: " << repo.toString();
    return false;
  }

  auto found_meta_it = metadata_map->find(role.ToString());
  if (found_meta_it == metadata_map->end()) {
    LOG_ERROR << "There are no any metadata for the given type of role: " << repo.toString() << ": " << role.ToString();
    return false;
  }

  if (role == Uptane::Role::Root() && version != Uptane::Version()) {
    // If requesting a Root version beyond what we have available, fail as
    // expected. If requesting a version before what is available, just use what
    // is available, since root rotation isn't supported here.
    if (repo == Uptane::RepositoryType::Director() && director_root_version < version) {
      LOG_DEBUG << "Requested Director root version " << version << " but only version " << director_root_version
                << " is available.";
      return false;
    } else if (repo == Uptane::RepositoryType::Image() && image_root_version < version) {
      LOG_DEBUG << "Requested Image repo root version " << version << " but only version " << image_root_version
                << " is available.";
      return false;
    }
  }

  *result = found_meta_it->second;
  return true;
}
