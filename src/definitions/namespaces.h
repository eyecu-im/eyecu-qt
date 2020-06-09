#ifndef DEF_NAMESPACES_H
#define DEF_NAMESPACES_H

#define NS_XML                                  "http://www.w3.org/XML/1998/namespace"

#define NS_XMPP_DELAY                           "urn:xmpp:delay"
#define NS_XMPP_TIME                            "urn:xmpp:time"
#define NS_XMPP_PING                            "urn:xmpp:ping"
#define NS_STANZA_SESSION                       "urn:xmpp:ssn"
#define NS_CAPTCHA_FORMS                        "urn:xmpp:captcha"
#define NS_BITS_OF_BINARY                       "urn:xmpp:bob"

#define NS_JABBER_CLIENT                        "jabber:client"
#define NS_JABBER_STREAMS                       "http://etherx.jabber.org/streams"
#define NS_JABBER_IQ_AUTH                       "jabber:iq:auth"
#define NS_JABBER_ROSTER                        "jabber:iq:roster"
#define NS_JABBER_PRIVATE                       "jabber:iq:private"
#define NS_JABBER_VERSION                       "jabber:iq:version"
#define NS_JABBER_LAST                          "jabber:iq:last"
#define NS_JABBER_DELAY                         "jabber:x:delay"
#define NS_JABBER_DATA                          "jabber:x:data"
#define NS_JABBER_REGISTER                      "jabber:iq:register"
#define NS_JABBER_OOB_X                         "jabber:x:oob"
#define NS_JABBER_OOB_IQ                        "jabber:iq:oob"
#define NS_JABBER_SEARCH                        "jabber:iq:search"
#define NS_JABBER_GATEWAY                       "jabber:iq:gateway"
#define NS_JABBER_PRIVACY                       "jabber:iq:privacy"
#define NS_JABBER_X_AVATAR                      "jabber:x:avatar"
#define NS_JABBER_IQ_AVATAR                     "jabber:iq:avatar"
#define NS_JABBER_X_CONFERENCE                  "jabber:x:conference"

#define NS_VCARD_TEMP                           "vcard-temp"
#define NS_VCARD_UPDATE                         "vcard-temp:x:update"

#define NS_STORAGE_GROUP_DELIMITER              "roster:delimiter"
#define NS_STORAGE_BOOKMARKS                    "storage:bookmarks"
#define NS_STORAGE_METACONTACTS                 "vacuum:metacontacts"

#define NS_FEATURE_IQAUTH                       "http://jabber.org/features/iq-auth"
#define NS_FEATURE_SASL                         "urn:ietf:params:xml:ns:xmpp-sasl"
#define NS_FEATURE_BIND                         "urn:ietf:params:xml:ns:xmpp-bind"
#define NS_FEATURE_SESSION                      "urn:ietf:params:xml:ns:xmpp-session"
#define NS_FEATURE_COMPRESS                     "http://jabber.org/features/compress"
#define NS_PROTOCOL_COMPRESS                    "http://jabber.org/protocol/compress"
#define NS_FEATURE_STARTTLS                     "urn:ietf:params:xml:ns:xmpp-tls"
#define NS_FEATURE_REGISTER                     "http://jabber.org/features/iq-register"
#define NS_FEATURE_ROSTER_VER                   "urn:xmpp:features:rosterver"

#define NS_MUC                                  "http://jabber.org/protocol/muc"
#define NS_MUC_USER                             "http://jabber.org/protocol/muc#user"
#define NS_MUC_ADMIN                            "http://jabber.org/protocol/muc#admin"
#define NS_MUC_OWNER                            "http://jabber.org/protocol/muc#owner"

#define NS_DISCO                                "http://jabber.org/protocol/disco"
#define NS_DISCO_INFO                           "http://jabber.org/protocol/disco#info"
#define NS_DISCO_ITEMS                          "http://jabber.org/protocol/disco#items"
#define NS_DISCO_PUBLISH                        "http://jabber.org/protocol/disco#publish"

#define NS_JABBER_XDATALAYOUT                   "http://jabber.org/protocol/xdata-layout"
#define NS_JABBER_XDATAVALIDATE                 "http://jabber.org/protocol/xdata-validate"
#define NS_XMPP_MEDIA_ELEMENT                   "urn:xmpp:media-element"

#define NS_CAPS                                 "http://jabber.org/protocol/caps"

#define NS_COMMANDS                             "http://jabber.org/protocol/commands"

#define NS_ARCHIVE                              "urn:xmpp:archive"
#define NS_ARCHIVE_AUTO                         "urn:xmpp:archive:auto"
#define NS_ARCHIVE_MANAGE                       "urn:xmpp:archive:manage"
#define NS_ARCHIVE_MANUAL                       "urn:xmpp:archive:manual"
#define NS_ARCHIVE_PREF                         "urn:xmpp:archive:pref"

#define NS_RESULTSET                            "http://jabber.org/protocol/rsm"

#define NS_FEATURENEG                           "http://jabber.org/protocol/feature-neg"

#define NS_CHATSTATES                           "http://jabber.org/protocol/chatstates"

#define NS_STREAM_INITIATION                    "http://jabber.org/protocol/si"
#define NS_STREAM_PUBLICATION                   "http://jabber.org/protocol/sipub"
#define NS_SI_FILETRANSFER                      "http://jabber.org/protocol/si/profile/file-transfer"

#define NS_INBAND_BYTESTREAMS                   "http://jabber.org/protocol/ibb"
#define NS_SOCKS5_BYTESTREAMS                   "http://jabber.org/protocol/bytestreams"

#define NS_PUBSUB                               "http://jabber.org/protocol/pubsub"
#define NS_PUBSUB_ERRORS                        "http://jabber.org/protocol/pubsub#errors"
#define NS_PUBSUB_EVENT                         "http://jabber.org/protocol/pubsub#event"
#define NS_PUBSUB_OWNER                         "http://jabber.org/protocol/pubsub#owner"

#define NS_ADDRESS                              "http://jabber.org/protocol/address"

#define NS_ROSTERX                              "http://jabber.org/protocol/rosterx"

#define NS_MESSAGE_CARBONS                      "urn:xmpp:carbons:2"
#define NS_MESSAGE_FORWARD                      "urn:xmpp:forward:0"

#define NS_VACUUM_PRIVATESTORAGE_UPDATE         "vacuum:privatestorage:update"

// *** <<< eyeCU <<< ***
#define NS_EYECU								"http://eyecu.ru"

#define NS_PUBSUB_RETRACT                       "http://jabber.org/protocol/pubsub#retract"

#define NS_PEP_ACTIVITY                         "http://jabber.org/protocol/activity"
#define NS_PEP_MOOD                             "http://jabber.org/protocol/mood"
#define NS_PEP_TUNE                             "http://jabber.org/protocol/tune"
#define NS_PEP_NICK                             "http://jabber.org/protocol/nick"

//SCE
#define NS_SCE									"urn:xmpp:sce:0"
//OMEMO
//#define NS_OMEMO								"eu.siacs.conversations.axolotl"
#define NS_OMEMO								"urn:xmpp:omemo:1"
#define NS_PEP_OMEMO							NS_OMEMO":devices"
#define NS_PEP_OMEMO_BUNDLES					NS_OMEMO":bundles"
#define NS_PEP_OMEMO_NOTIFY						NS_PEP_OMEMO"+notify"

#define NS_RECEIPTS                         	"urn:xmpp:receipts"
#define NS_ATTENTION                         	"urn:xmpp:attention:0"
#define NS_CHATMARKERS                          "urn:xmpp:chat-markers:0"
#define NS_HINTS								"urn:xmpp:hints"
#define NS_EME									"urn:xmpp:eme:0"
#define NS_OTR									"urn:xmpp:otr:0"
#define NS_XHTML								"http://www.w3.org/1999/xhtml"
#define NS_XHTML_IM								"http://jabber.org/protocol/xhtml-im"
#define NS_X_POI                                "jabber:x:poi"

//Jingle
#define NS_JINGLE                               "urn:xmpp:jingle:1"
#define NS_JINGLE_APPS_RTP                      "urn:xmpp:jingle:apps:rtp:1"
#define NS_JINGLE_APPS_RTP_INFO                 "urn:xmpp:jingle:apps:rtp:info:1"
#define NS_JINGLE_APPS_RTP_AUDIO                "urn:xmpp:jingle:apps:rtp:audio"
#define NS_JINGLE_APPS_RTP_VIDEO                "urn:xmpp:jingle:apps:rtp:video"

#define NS_JINGLE_TRANSPORTS_RAW_UDP            "urn:xmpp:jingle:transports:raw-udp:1"
#define NS_JINGLE_TRANSPORTS_ICE_UDP            "urn:xmpp:jingle:transports:ice-udp:1"

//Other
#define NS_XMPP_STANZAS                         "urn:ietf:params:xml:ns:xmpp-stanzas"
// *** >>> eyeCU >>> ***

#endif // DEF_NAMESPACES_H
