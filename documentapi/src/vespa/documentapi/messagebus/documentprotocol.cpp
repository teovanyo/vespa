// Copyright Vespa.ai. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "replymerger.h"
#include "routable_factories_8.h"
#include "routablefactories60.h"
#include "routablerepository.h"
#include "routingpolicyfactories.h"
#include "routingpolicyrepository.h"
#include <vespa/document/util/stringutil.h>
#include <vespa/document/repo/documenttyperepo.h>
#include <vespa/documentapi/documentapi.h>
#include <vespa/messagebus/error.h>
#include <vespa/vespalib/util/exceptions.h>
#include <cassert>
#include <sstream>

#include <vespa/log/log.h>
LOG_SETUP(".documentprotocol");

using document::DocumentTypeRepo;

namespace documentapi {

const mbus::string DocumentProtocol::NAME = "document";

DocumentProtocol::DocumentProtocol(std::shared_ptr<const DocumentTypeRepo> repo, const string &configId) :
    _routingPolicyRepository(std::make_unique<RoutingPolicyRepository>()),
    _routableRepository(std::make_unique<RoutableRepository>()),
    _repo(std::move(repo))
{
    // Prepare config string for routing policy factories.
    string cfg = (configId.empty() ? "client" : configId);

    // When adding factories to this list, please KEEP THEM ORDERED alphabetically like they are now.
    putRoutingPolicyFactory("AND", std::make_shared<RoutingPolicyFactories::AndPolicyFactory>());
    putRoutingPolicyFactory("Content", std::make_shared<RoutingPolicyFactories::ContentPolicyFactory>());
    putRoutingPolicyFactory("DocumentRouteSelector", std::make_shared<RoutingPolicyFactories::DocumentRouteSelectorPolicyFactory>(*_repo, cfg));
    putRoutingPolicyFactory("Extern", std::make_shared<RoutingPolicyFactories::ExternPolicyFactory>());
    putRoutingPolicyFactory("LoadBalancer", std::make_shared<RoutingPolicyFactories::LoadBalancerPolicyFactory>());
    putRoutingPolicyFactory("LocalService", std::make_shared<RoutingPolicyFactories::LocalServicePolicyFactory>());
    putRoutingPolicyFactory("MessageType", std::make_shared<RoutingPolicyFactories::MessageTypePolicyFactory>());
    putRoutingPolicyFactory("RoundRobin", std::make_shared<RoutingPolicyFactories::RoundRobinPolicyFactory>());
    putRoutingPolicyFactory("SubsetService", std::make_shared<RoutingPolicyFactories::SubsetServicePolicyFactory>());

    add_legacy_v6_factories();
    add_v8_factories();
}

DocumentProtocol::~DocumentProtocol() = default;

void
DocumentProtocol::add_legacy_v6_factories()
{
    // Prepare version specifications to use when adding routable factories.
    vespalib::VersionSpecification version6(6, 221);
    std::vector<vespalib::VersionSpecification> from6  = { version6 };

    // Add 6.x serialization
    putRoutableFactory(MESSAGE_CREATEVISITOR,   std::make_shared<RoutableFactories60::CreateVisitorMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_DESTROYVISITOR,  std::make_shared<RoutableFactories60::DestroyVisitorMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_DOCUMENTLIST,    std::make_shared<RoutableFactories60::DocumentListMessageFactory>(*_repo), from6);
    putRoutableFactory(MESSAGE_EMPTYBUCKETS,    std::make_shared<RoutableFactories60::EmptyBucketsMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_GETBUCKETLIST,   std::make_shared<RoutableFactories60::GetBucketListMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_GETBUCKETSTATE,  std::make_shared<RoutableFactories60::GetBucketStateMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_GETDOCUMENT,     std::make_shared<RoutableFactories60::GetDocumentMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_MAPVISITOR,      std::make_shared<RoutableFactories60::MapVisitorMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_PUTDOCUMENT,     std::make_shared<RoutableFactories60::PutDocumentMessageFactory>(*_repo), from6);
    putRoutableFactory(MESSAGE_QUERYRESULT,     std::make_shared<RoutableFactories60::QueryResultMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_REMOVEDOCUMENT,  std::make_shared<RoutableFactories60::RemoveDocumentMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_REMOVELOCATION,  std::make_shared<RoutableFactories60::RemoveLocationMessageFactory>(*_repo), from6);
    putRoutableFactory(MESSAGE_STATBUCKET,      std::make_shared<RoutableFactories60::StatBucketMessageFactory>(), from6);
    putRoutableFactory(MESSAGE_UPDATEDOCUMENT,  std::make_shared<RoutableFactories60::UpdateDocumentMessageFactory>(*_repo), from6);
    putRoutableFactory(MESSAGE_VISITORINFO,     std::make_shared<RoutableFactories60::VisitorInfoMessageFactory>(), from6);
    putRoutableFactory(REPLY_CREATEVISITOR,     std::make_shared<RoutableFactories60::CreateVisitorReplyFactory>(), from6);
    putRoutableFactory(REPLY_DESTROYVISITOR,    std::make_shared<RoutableFactories60::DestroyVisitorReplyFactory>(), from6);
    putRoutableFactory(REPLY_DOCUMENTIGNORED,   std::make_shared<RoutableFactories60::DocumentIgnoredReplyFactory>(), from6);
    putRoutableFactory(REPLY_DOCUMENTLIST,      std::make_shared<RoutableFactories60::DocumentListReplyFactory>(), from6);
    putRoutableFactory(REPLY_EMPTYBUCKETS,      std::make_shared<RoutableFactories60::EmptyBucketsReplyFactory>(), from6);
    putRoutableFactory(REPLY_GETBUCKETLIST,     std::make_shared<RoutableFactories60::GetBucketListReplyFactory>(), from6);
    putRoutableFactory(REPLY_GETBUCKETSTATE,    std::make_shared<RoutableFactories60::GetBucketStateReplyFactory>(), from6);
    putRoutableFactory(REPLY_GETDOCUMENT,       std::make_shared<RoutableFactories60::GetDocumentReplyFactory>(*_repo), from6);
    putRoutableFactory(REPLY_MAPVISITOR,        std::make_shared<RoutableFactories60::MapVisitorReplyFactory>(), from6);
    putRoutableFactory(REPLY_PUTDOCUMENT,       std::make_shared<RoutableFactories60::PutDocumentReplyFactory>(), from6);
    putRoutableFactory(REPLY_QUERYRESULT,       std::make_shared<RoutableFactories60::QueryResultReplyFactory>(), from6);
    putRoutableFactory(REPLY_REMOVEDOCUMENT,    std::make_shared<RoutableFactories60::RemoveDocumentReplyFactory>(), from6);
    putRoutableFactory(REPLY_REMOVELOCATION,    std::make_shared<RoutableFactories60::RemoveLocationReplyFactory>(), from6);
    putRoutableFactory(REPLY_STATBUCKET,        std::make_shared<RoutableFactories60::StatBucketReplyFactory>(), from6);
    putRoutableFactory(REPLY_UPDATEDOCUMENT,    std::make_shared<RoutableFactories60::UpdateDocumentReplyFactory>(), from6);
    putRoutableFactory(REPLY_VISITORINFO,       std::make_shared<RoutableFactories60::VisitorInfoReplyFactory>(), from6);
    putRoutableFactory(REPLY_WRONGDISTRIBUTION, std::make_shared<RoutableFactories60::WrongDistributionReplyFactory>(), from6);
}

void
DocumentProtocol::add_v8_factories()
{
    vespalib::VersionSpecification version8(8, 310);
    std::vector<vespalib::VersionSpecification> from8 = { version8 };

    using RF8 = messagebus::RoutableFactories80;
    auto put_v8_factory = [&](auto msg_id, auto factory) {
        putRoutableFactory(msg_id, std::move(factory), from8);
    };

    put_v8_factory(MESSAGE_CREATEVISITOR,   RF8::create_visitor_message_factory());
    put_v8_factory(MESSAGE_DESTROYVISITOR,  RF8::destroy_visitor_message_factory());
    put_v8_factory(MESSAGE_DOCUMENTLIST,    RF8::document_list_message_factory(_repo));
    put_v8_factory(MESSAGE_EMPTYBUCKETS,    RF8::empty_buckets_message_factory());
    put_v8_factory(MESSAGE_GETBUCKETLIST,   RF8::get_bucket_list_message_factory());
    put_v8_factory(MESSAGE_GETBUCKETSTATE,  RF8::get_bucket_state_message_factory());
    put_v8_factory(MESSAGE_GETDOCUMENT,     RF8::get_document_message_factory());
    put_v8_factory(MESSAGE_MAPVISITOR,      RF8::map_visitor_message_factory());
    put_v8_factory(MESSAGE_PUTDOCUMENT,     RF8::put_document_message_factory(_repo));
    put_v8_factory(MESSAGE_QUERYRESULT,     RF8::query_result_message_factory());
    put_v8_factory(MESSAGE_REMOVEDOCUMENT,  RF8::remove_document_message_factory());
    put_v8_factory(MESSAGE_REMOVELOCATION,  RF8::remove_location_message_factory(_repo));
    put_v8_factory(MESSAGE_STATBUCKET,      RF8::stat_bucket_message_factory());
    put_v8_factory(MESSAGE_UPDATEDOCUMENT,  RF8::update_document_message_factory(_repo));
    put_v8_factory(MESSAGE_VISITORINFO,     RF8::visitor_info_message_factory());
    put_v8_factory(REPLY_CREATEVISITOR,     RF8::create_visitor_reply_factory());
    put_v8_factory(REPLY_DESTROYVISITOR,    RF8::destroy_visitor_reply_factory());
    put_v8_factory(REPLY_DOCUMENTIGNORED,   RF8::document_ignored_reply_factory());
    put_v8_factory(REPLY_DOCUMENTLIST,      RF8::document_list_reply_factory());
    put_v8_factory(REPLY_EMPTYBUCKETS,      RF8::empty_buckets_reply_factory());
    put_v8_factory(REPLY_GETBUCKETLIST,     RF8::get_bucket_list_reply_factory());
    put_v8_factory(REPLY_GETBUCKETSTATE,    RF8::get_bucket_state_reply_factory());
    put_v8_factory(REPLY_GETDOCUMENT,       RF8::get_document_reply_factory(_repo));
    put_v8_factory(REPLY_MAPVISITOR,        RF8::map_visitor_reply_factory());
    put_v8_factory(REPLY_PUTDOCUMENT,       RF8::put_document_reply_factory());
    put_v8_factory(REPLY_QUERYRESULT,       RF8::query_result_reply_factory());
    put_v8_factory(REPLY_REMOVEDOCUMENT,    RF8::remove_document_reply_factory());
    put_v8_factory(REPLY_REMOVELOCATION,    RF8::remove_location_reply_factory());
    put_v8_factory(REPLY_STATBUCKET,        RF8::stat_bucket_reply_factory());
    put_v8_factory(REPLY_UPDATEDOCUMENT,    RF8::update_document_reply_factory());
    put_v8_factory(REPLY_VISITORINFO,       RF8::visitor_info_reply_factory());
    put_v8_factory(REPLY_WRONGDISTRIBUTION, RF8::wrong_distribution_reply_factory());
}

mbus::IRoutingPolicy::UP
DocumentProtocol::createPolicy(const mbus::string &name, const mbus::string &param) const
{
    return _routingPolicyRepository->createPolicy(name, param);
}

DocumentProtocol &
DocumentProtocol::putRoutingPolicyFactory(const string &name, IRoutingPolicyFactory::SP factory)
{
    _routingPolicyRepository->putFactory(name, std::move(factory));
    return *this;
}

mbus::Blob
DocumentProtocol::encode(const vespalib::Version &version, const mbus::Routable &routable) const
{
    mbus::Blob blob(_routableRepository->encode(version, routable));
    if (LOG_WOULD_LOG(spam)) {
        std::ostringstream message;
        document::StringUtil::printAsHex(message, blob.data(), blob.size());
        LOG(spam, "Encoded message of protocol %s type %u using version %s serialization:\n%s",
            routable.getProtocol().c_str(), routable.getType(),
            version.toAbbreviatedString().c_str(), message.str().c_str());
    }
    return blob;
}

mbus::Routable::UP
DocumentProtocol::decode(const vespalib::Version &version, mbus::BlobRef data) const
{
    try {
        return _routableRepository->decode(version, data);
    } catch (vespalib::Exception &e) {
        LOG(warning, "%s", e.getMessage().c_str());
        return {};
    }
}

uint32_t
DocumentProtocol::getRoutableTypes(const vespalib::Version &version, std::vector<uint32_t> &out) const
{
    return _routableRepository->getRoutableTypes(version, out);
}

DocumentProtocol &
DocumentProtocol::putRoutableFactory(uint32_t type, IRoutableFactory::SP factory,
                                     const vespalib::VersionSpecification &version)
{
    _routableRepository->putFactory(version, type, std::move(factory));
    return *this;
}

DocumentProtocol &
DocumentProtocol::putRoutableFactory(uint32_t type, IRoutableFactory::SP factory,
                                     const std::vector<vespalib::VersionSpecification> &versions)
{
    for (const auto & version : versions) {
        putRoutableFactory(type, factory, version);
    }
    return *this;
}

string
DocumentProtocol::getErrorName(uint32_t errorCode) {
    switch (errorCode) {
        case ERROR_MESSAGE_IGNORED:               return "MESSAGE_IGNORED";
        case ERROR_POLICY_FAILURE:                return "POLICY_FAILURE";
        case ERROR_DOCUMENT_NOT_FOUND:            return "DOCUMENT_NOT_FOUND";
        case ERROR_EXISTS:                        return "EXISTS";
        case ERROR_BUCKET_NOT_FOUND:              return "BUCKET_NOT_FOUND";
        case ERROR_BUCKET_DELETED:                return "BUCKET_DELETED";
        case ERROR_NOT_IMPLEMENTED:               return "NOT_IMPLEMENTED";
        case ERROR_ILLEGAL_PARAMETERS:            return "ILLEGAL_PARAMETERS";
        case ERROR_IGNORED:                       return "IGNORED";
        case ERROR_UNKNOWN_COMMAND:               return "UNKNOWN_COMMAND";
        case ERROR_UNPARSEABLE:                   return "UNPARSEABLE";
        case ERROR_NO_SPACE:                      return "NO_SPACE";
        case ERROR_INTERNAL_FAILURE:              return "INTERNAL_FAILURE";
        case ERROR_PROCESSING_FAILURE:            return "PROCESSING_FAILURE";
        case ERROR_TIMESTAMP_EXIST:               return "TIMESTAMP_EXIST";
        case ERROR_STALE_TIMESTAMP:               return "STALE_TIMESTAMP";
        case ERROR_NODE_NOT_READY:                return "NODE_NOT_READY";
        case ERROR_WRONG_DISTRIBUTION:            return "WRONG_DISTRIBUTION";
        case ERROR_REJECTED:                      return "REJECTED";
        case ERROR_ABORTED:                       return "ABORTED";
        case ERROR_BUSY:                          return "BUSY";
        case ERROR_NOT_CONNECTED:                 return "NOT_CONNECTED";
        case ERROR_DISK_FAILURE:                  return "DISK_FAILURE";
        case ERROR_IO_FAILURE:                    return "IO_FAILURE";
        case ERROR_SUSPENDED:                     return "SUSPENDED";
        case ERROR_TEST_AND_SET_CONDITION_FAILED: return "TEST_AND_SET_CONDITION_FAILED";
    }
    return mbus::ErrorCode::getName(errorCode);
}

void
DocumentProtocol::merge(mbus::RoutingContext &ctx)
{
    std::set<uint32_t> mask;
    merge(ctx, mask);
}

void
DocumentProtocol::merge(mbus::RoutingContext& ctx,
                        const std::set<uint32_t>& mask)
{
    ReplyMerger rm;
    uint32_t idx = 0;
    for (mbus::RoutingNodeIterator it = ctx.getChildIterator();
         it.isValid(); it.next(), ++idx)
    {
        if (mask.find(idx) != mask.end()) {
            continue;
        }
        rm.merge(idx, it.getReplyRef());
    }
    assert(idx != 0);
    ReplyMerger::Result res(rm.mergedReply());
    if (res.isSuccessful()) {
        const uint32_t okIdx = res.getSuccessfulReplyIndex();
        ctx.setReply(ctx.getChildIterator().skip(okIdx).removeReply()); 
    } else {
        assert(res.hasGeneratedReply());
        ctx.setReply(res.releaseGeneratedReply());
    }
}

bool
DocumentProtocol::hasOnlyErrorsOfType(const mbus::Reply &reply, uint32_t errCode)
{
    for (uint32_t i = 0; i < reply.getNumErrors(); ++i) {
        if (reply.getError(i).getCode() != errCode) {
            return false;
        }
    }
    return true;
}

}
