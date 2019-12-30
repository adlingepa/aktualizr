#include "aktualizr_secondary_factory.h"

#include "crypto/keymanager.h"
#include "update_agent_file.h"

#ifdef BUILD_OSTREE
#include "package_manager/ostreemanager.h"
#include "update_agent_ostree.h"
#endif

// TODO: consider implementation of a proper registry/builder/factory
AktualizrSecondary::Ptr AktualizrSecondaryFactory::create(const AktualizrSecondaryConfig& config) {
  auto storage = INvStorage::newStorage(config.storage);
  return AktualizrSecondaryFactory::create(config, storage);
}

AktualizrSecondary::Ptr AktualizrSecondaryFactory::create(const AktualizrSecondaryConfig& config,
                                                          const std::shared_ptr<INvStorage>& storage) {
  auto bootloader = std::make_shared<Bootloader>(config.bootloader, storage);
  auto key_mngr = std::make_shared<KeyManager>(storage, config.keymanagerConfig());

  UpdateAgent::Ptr update_agent;

  if (config.pacman.type != PackageManager::kOstree) {
    update_agent = std::make_shared<FileUpdateAgent>(config.pacman.target_name, config.pacman.filepath.string());
  }
#ifdef BUILD_OSTREE
  else {
    std::shared_ptr<OstreeManager> pack_man =
        std::make_shared<OstreeManager>(config.pacman, bootloader, storage, nullptr);
    update_agent =
        std::make_shared<OstreeUpdateAgent>(config.pacman.sysroot, key_mngr, pack_man, config.uptane.ecu_hardware_id);
  }
#endif

  return std::make_shared<AktualizrSecondary>(config, storage, bootloader, key_mngr, update_agent);
}
