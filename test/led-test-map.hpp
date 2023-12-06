#include "ledlayout.hpp"

static const phosphor::led::GroupMap singleLedOn = {
        {"/xyz/openbmc_project/ledmanager/groups/SingleLed",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
         }},
};

static const phosphor::led::GroupMap singleLedBlink = {
        {"/xyz/openbmc_project/ledmanager/groups/SingleLed",
         {
             {"One", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::Blink},
         }},
};

static const phosphor::led::GroupMap singleLedBlinkOverrideOn = {
        {"/xyz/openbmc_project/ledmanager/groups/SingleLed",
         {
             {"One", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap multipleLedsOn = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLeds",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap multipleLedsBlink = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLeds",
         {
             {"One", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::Blink},
             {"Two", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::Blink},
             {"Three", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::Blink},
         }},
};

static const phosphor::led::GroupMap multipleLedsOnAndBlink = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsMix",
         {
             {"One", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::Blink},
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Three", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::On},
             {"Four", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Five", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
         }},
};

static const phosphor::led::GroupMap twoGroupsWithDistinctLEDsOn = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsASet",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
         }},
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsBSet",
         {
             {"Four", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Five", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Six", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap twoGroupsWithOneComonLEDOn = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsASet",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsBSet",
         {
             {"Four", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Six", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap
    twoGroupsWithOneComonLEDOnOneLEDBlinkPriority = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsASet",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::Blink},
         }},
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsBSet",
         {
             {"Four", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Six", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap twoGroupsWithOneComonLEDOnPriority = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsASet",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::On},
         }},
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsBSet",
         {
             {"Four", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Six", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap twoGroupsWithMultiplComonLEDOn = {        
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsASet",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsBSet",
         {
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Six", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Seven", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap
    twoGroupsWithMultipleComonLEDInDifferentState = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsASet",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Two", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::On},
             {"Four", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsBSet",
         {
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::On},
             {"Five", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Six", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
         }},
};

static const phosphor::led::GroupMap
    twoGroupsWithMultipleComonLEDInDifferentStateDiffPriority = {
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsASet",
         {
             {"One", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Two", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Four", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Ten", phosphor::led::Layout::Blink, 0, 0,
              phosphor::led::Layout::Blink},
         }},
        {"/xyz/openbmc_project/ledmanager/groups/MultipleLedsBSet",
         {
             {"Two", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Three", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
             {"Five", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Six", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::On},
             {"Ten", phosphor::led::Layout::On, 0, 0,
              phosphor::led::Layout::Blink},
         }},
};
