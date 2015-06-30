#ifndef DEF_ROSTERLABELS_H
#define DEF_ROSTERLABELS_H

/*** <<< eyeCU <<< ***/
#define FLOOR_200   200
#define FLOOR_128   128

#define ADI_MC      AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter
#define ADI_MR      AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleRight
#define ADI_ML      AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleLeft
#define ADI_TOP     AdvancedDelegateItem::makeId(AdvancedDelegateItem::Top
#define ADI_BOT     AdvancedDelegateItem::makeId(AdvancedDelegateItem::Bottom
/*** >>> eyeCU >>> ***/

#define ADI_AL_R    AdvancedDelegateItem::AlignRightOrderMask
//Position=MiddleCenter; Floor=128
#define RLID_METACONTACTS_BRANCH                   AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,200)
#define RLID_ROSTERSVIEW_RESOURCES                 AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,700)
#define RLID_SCHANGER_CONNECTING                   AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,AdvancedDelegateItem::AlignRightOrderMask | 100)
#define RLID_BIRTHDAY_NOTIFY                       AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,AdvancedDelegateItem::AlignRightOrderMask | 200)
#define RLID_PRIVACY_STATUS                        AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,AdvancedDelegateItem::AlignRightOrderMask | 300)
#define RLID_CONNECTION_ENCRYPTED                  AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,AdvancedDelegateItem::AlignRightOrderMask | 500)
#define RLID_RECENT_FAVORITE                       AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,AdvancedDelegateItem::AlignRightOrderMask | 1000)
#define RLID_RECENT_INSERT_FAVORITE                AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,AdvancedDelegateItem::AlignRightOrderMask | 1001)
#define RLID_RECENT_REMOVE_FAVORITE                AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,128,AdvancedDelegateItem::AlignRightOrderMask | 1002)

//Position=MiddleCenter; Floor=200
#define RLID_ROSTERSVIEW_STATUS                    AdvancedDelegateItem::makeId(AdvancedDelegateItem::MiddleCenter,200,500)

//Position=MiddleRight; Floor=128
/*** <<< eyeCU <<< ***/
#define RLID_ACTIVITY                              ADI_MC,FLOOR_128,ADI_AL_R | 2000)
#define RLID_MOOD                                  ADI_MC,FLOOR_128,ADI_AL_R | 2010)
#define RLID_TUNE                                  ADI_MC,FLOOR_128,ADI_AL_R | 2020)
#define RLID_GEOLOC                                ADI_MC,FLOOR_128,ADI_AL_R | 2030)
#define RLID_CLIENTICONS                           ADI_MC,FLOOR_128,ADI_AL_R | 2040)
#define RLID_AVATAR_IMAGE_LEFT                     ADI_ML,FLOOR_128, 50)
#define RLID_AVATAR_IMAGE_RIGHT                    ADI_MR,FLOOR_200, 500)
/*** >>> eyeCU >>> ***/

#endif //DEF_ROSTERLABELS_H
