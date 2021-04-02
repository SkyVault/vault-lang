#pragma once

#include <string_view>

#define VAULT_VERSION_MAJOR "0"
#define VAULT_VERSION_MINOR "0"
#define VAULT_VERSION_PATCH "0" 

#define VAULT_VERSION "v" VAULT_VERSION_MAJOR "." VAULT_VERSION_MINOR "." VAULT_VERSION_PATCH

#define VAULT_PROMPT "> "

namespace Vault {
  enum Status {
    FAILED,
    SUCCESS,
  };
}