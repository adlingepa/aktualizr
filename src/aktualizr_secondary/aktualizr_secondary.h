#ifndef AKTUALIZR_SECONDARY_H
#define AKTUALIZR_SECONDARY_H

#include "aktualizr_secondary_config.h"
#include "aktualizr_secondary_metadata.h"
#include "uptane/secondaryinterface.h"

#include "uptane/directorrepository.h"
#include "uptane/imagerepository.h"
#include "uptane/manifest.h"
#include "asn1/asn1_message.h"

class UpdateAgent;
class INvStorage;
class KeyManager;
class HandlerMap;

class AktualizrSecondary : public Uptane::SecondaryInterface {
 public:
  using Ptr = std::shared_ptr<AktualizrSecondary>;

 public:
  AktualizrSecondary(AktualizrSecondaryConfig config, std::shared_ptr<INvStorage> storage,
                     std::shared_ptr<KeyManager> key_mngr, std::shared_ptr<UpdateAgent> update_agent);

  std::string Type() const override { return ""; }
  Uptane::EcuSerial getSerial() const override;
  Uptane::HardwareIdentifier getHwId() const override;
  PublicKey getPublicKey() const override;

  Uptane::Manifest getManifest() const override;
  bool ping() const override { return true; }
  bool putMetadata(const Uptane::RawMetaPack& meta_pack) override { return putMetadata(Metadata(meta_pack)); }
  int32_t getRootVersion(bool director) const override;
  bool putRoot(const std::string& root, bool director) override;
  virtual data::ResultCode::Numeric install(const Uptane::Target& target_name) override;
  bool sendFirmware(const std::string& firmware);
  data::ResultCode::Numeric install(const std::string& target_name);

  void completeInstall();
  bool putMetadata(const Metadata& metadata);
  HandlerMap& getHandlerMap() const {
    return *handler_map_;
  }

 private:
  bool hasPendingUpdate() { return storage_->hasPendingInstall(); }
  bool doFullVerification(const Metadata& metadata);
  void uptaneInitialize();
  void initPendingTargetIfAny();
  void initHandlerMap();

  Asn1Message::Ptr download(Asn1Message::Ptr);

 private:
  Uptane::DirectorRepository director_repo_;
  Uptane::ImageRepository image_repo_;

  Uptane::Target pending_target_{Uptane::Target::Unknown()};

  AktualizrSecondaryConfig config_;
  std::shared_ptr<INvStorage> storage_;
  std::shared_ptr<KeyManager> keys_;
  Uptane::ManifestIssuer::Ptr manifest_issuer_;

  Uptane::EcuSerial ecu_serial_{Uptane::EcuSerial::Unknown()};
  Uptane::HardwareIdentifier hardware_id_{Uptane::HardwareIdentifier::Unknown()};

  std::shared_ptr<UpdateAgent> update_agent_;
  std::shared_ptr<HandlerMap> handler_map_;

};

#endif  // AKTUALIZR_SECONDARY_H
