#ifndef OSTREE_H_
#define OSTREE_H_

#include <memory>
#include <string>

#include <glib/gi18n.h>
#include <ostree-1/ostree.h>

#include "crypto/keymanager.h"
#include "packagemanagerconfig.h"
#include "packagemanagerinterface.h"
#include "utilities/events.h"
#include "utilities/types.h"

const char remote[] = "aktualizr-remote";

template <typename T>
struct GObjectFinalizer {
  void operator()(T *e) const { g_object_unref(reinterpret_cast<gpointer>(e)); }
};

struct PullMetaStruct {
  PullMetaStruct(Uptane::Target target_in, std::shared_ptr<event::Channel> events_channel_in)
      : target{std::move(target_in)}, percent_complete{0}, events_channel{std::move(events_channel_in)} {}
  Uptane::Target target;
  unsigned int percent_complete;
  std::shared_ptr<event::Channel> events_channel;
};

using OstreeDeploymentPtr = std::unique_ptr<OstreeDeployment, GObjectFinalizer<OstreeDeployment>>;
using OstreeSysrootPtr = std::unique_ptr<OstreeSysroot, GObjectFinalizer<OstreeSysroot>>;
using OstreeRepoPtr = std::unique_ptr<OstreeRepo, GObjectFinalizer<OstreeRepo>>;

class OstreeManager : public PackageManagerInterface {
 public:
  OstreeManager(PackageConfig pconfig, std::shared_ptr<INvStorage> storage);
  std::string name() override { return "ostree"; }
  Json::Value getInstalledPackages() override;
  Uptane::Target getCurrent() override;
  bool imageUpdated() override;
  data::InstallOutcome install(const Uptane::Target &target) const override;

  OstreeDeploymentPtr getStagedDeployment();
  static OstreeSysrootPtr LoadSysroot(const boost::filesystem::path &path);
  static OstreeRepoPtr LoadRepo(OstreeSysroot *sysroot, GError **error);
  static bool addRemote(OstreeRepo *repo, const std::string &url, const KeyManager &keys);
  static data::InstallOutcome pull(const boost::filesystem::path &sysroot_path, const std::string &ostree_server,
                                   const KeyManager &keys, const Uptane::Target &target,
                                   const std::shared_ptr<event::Channel> &events_channel = nullptr);

 private:
  PackageConfig config;
  std::shared_ptr<INvStorage> storage_;
};

#endif  // OSTREE_H_
