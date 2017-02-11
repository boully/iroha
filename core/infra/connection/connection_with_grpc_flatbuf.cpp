/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


#include "../../model/objects/base_object.hpp"

#include "../../consensus/connection/connection.hpp"
#include "../../consensus/consensus_event.hpp"
#include "../../util/logger.hpp"
#include "../../util/exception.hpp"
#include "../config/peer_service_with_json.hpp"

#include <iostream>
#include <thread>

#include <assert.h>
#include <grpc++/grpc++.h>
#include "flatbuffers/reflection.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"


#include "../../../flatbuf/api_generated.h"
#include "../../../flatbuf/api.grpc.fb.h"

namespace connection {
    using namespace iroha;

    template<typename T>
    using Offset = flatbuffers::Offset<T>;

    std::vector<std::string> receiver_ips;
    std::vector<
        std::function<void(
            const std::string& from,
            event::ConsensusEvent& message
        )>
    > receivers;

    auto rehasher = flatbuffers::rehasher_function_t(
      [](void *pointer) -> flatbuffers::hash_value_t {
          (void)pointer;
          return 0;
    });

    template<typename In,typename Out>
    Out decodeFlatbufferT(In& input){}

    std::pair<std::string, object::BaseObject> decodeFlatbufferT(iroha::BaseObjectT& obj){
      switch (obj.type) {
        case BaseObjectType::BaseObjectType_Text:{
          return std::make_pair( obj.name, object::BaseObject(obj.text));
        }
        case BaseObjectType::BaseObjectType_Integer:{
          return std::make_pair( obj.name, object::BaseObject(obj.integer));
        }
        case BaseObjectType::BaseObjectType_Boolean: {
          return std::make_pair( obj.name, object::BaseObject(obj.boolean));
        }
        case BaseObjectType::BaseObjectType_Decimal: {
          return std::make_pair( obj.name, object::BaseObject(obj.decimal));
        }
        default:{
          throw exception::NotImplementedException(
            "This BaseObject type is not implemented.",__FILE__
          );
        }
      }
    }

    object::Object decodeFlatbufferT(ObjectUnion& u){
      switch(u.type){
        case Object::Object_Asset:{
          auto asset = object::Asset();
          asset.domain = u.AsAsset()->domain;
          asset.name   = u.AsAsset()->name;
          for(auto&& o: u.AsAsset()->objects){
            asset.value.insert( decodeFlatbufferT(*o) );
          }
          return object::Object(asset);
        }
        case Object::Object_Domain:{
            throw exception::NotImplementedException(
              "This Object type is not implemented.",__FILE__
            );
        }
        case Object::Object_Account:{
            throw exception::NotImplementedException(
              "This Object type is not implemented.",__FILE__
            );
        }
        case Object::Object_Peer:{
            throw exception::NotImplementedException(
              "This Object type is not implemented.",__FILE__
            );
        }
        default:{
          throw exception::NotImplementedException(
            "This Object type is not implemented.",__FILE__
          );
        }
      }
    }

    command::Command decodeFlatbufferT(CommandUnion& u){
      switch(u.type){
        case Command::Command_Add:{
          auto object = decodeFlatbufferT(u.AsAdd()->object);
          return command::Command(command::Add(object));
        }
        case Command::Command_Transfer:{
          auto object = decodeFlatbufferT(u.AsTransfer()->object);
          return command::Command(command::Transfer(object,u.AsTransfer()->receiver));
        }
        case Command::Command_Contract:{
          auto object = decodeFlatbufferT(u.AsContract()->object);
          return command::Command(command::Contract(object,u.AsContract()->contractName));
        }
        case Command::Command_Remove:{
          auto object = decodeFlatbufferT(u.AsRemove()->object);
          return command::Command(command::Remove(object));
        }
        case Command::Command_Batch:{
          return command::Command(command::Batch());
        }
        case Command::Command_Unbatch:{
          return command::Command(command::Unbatch());
        }
        case Command::Command_Update:{
          auto object = decodeFlatbufferT(u.AsUpdate()->object);
          return command::Command(command::Update(object));
        }
        default:{
        }
      }
    }

    event::Transaction decodeFlatbufferT(TransactionT& u){
      event::Transaction res;

      return res;
    }

    event::ConsensusEvent decodeFlatbufferT(ConsensusEventT& u){
      if(u.transaction.empty()){
        throw exception::NotImplementedException(
          "Multi transaction supporting is not implemented.",__FILE__
        );
      }
      event::ConsensusEvent res( std::move(decodeFlatbufferT(*u.transaction.at(0))) );
      for(auto&& esig: u.eventSignatures){
        res.addEventSignature(std::move(esig->publicKey),std::move(esig->signature));
      }
      return res;
    }

    std::unique_ptr<iroha::BaseObjectT> encodeBaseObjectFlatbufferT(const std::string& name, const object::BaseObject& obj){
      std::unique_ptr<iroha::BaseObjectT> res(new iroha::BaseObjectT());

      res->name = name;
      res->text = "";res->integer = 0;res->boolean = false;res->decimal = 0.0f;

      if(obj.object_type == object::BaseObject::Object_type::STRING){
        res->text     = (std::string)obj;
      }else if(obj.object_type == object::BaseObject::Object_type::INTEGER){
        res->integer  = (int)obj;
      }else if(obj.object_type == object::BaseObject::Object_type::BOOLEAN){
        res->boolean  = (bool)obj;
      }else if(obj.object_type == object::BaseObject::Object_type::FLOAT){
        res->decimal  = (float)obj;
      }else{
        throw exception::NotImplementedException(
          "This base object type is not implemented.",__FILE__
        );
      }
      return std::move(res);
    }

    std::unique_ptr<ObjectUnion> encodeFlatbufferUnionT(const object::Object& obj) {

      std::unique_ptr<ObjectUnion> res(new ObjectUnion());

      if (obj.type == object::ObjectValueT::asset) {
        auto assetT = AssetT();
        assetT.domain = obj.asset->domain;
        assetT.name   = obj.asset->name;
        for (auto&& o: obj.asset->value) {
          assetT.objects.emplace_back(encodeBaseObjectFlatbufferT( o.first, o.second));
        }
        res->Set(std::move(assetT));
      } else if (obj.type == object::ObjectValueT::domain) {
        auto domainT = DomainT();
        throw exception::NotImplementedException(
          "This domain is not implemented.",__FILE__
        );
        res->Set(std::move(domainT));
      } else if (obj.type == object::ObjectValueT::account) {
        auto accountT = AccountT();
        throw exception::NotImplementedException(
          "This domain is not implemented.",__FILE__
        );
        res->Set(std::move(accountT));
      } else if (obj.type == object::ObjectValueT::peer) {
        auto peerT = PeerT();
        throw exception::NotImplementedException(
          "This domain is not implemented.",__FILE__
        );
        res->Set(std::move(peerT));
      } else {
        throw exception::NotImplementedException(
          "This object is not implemented.",__FILE__
        );
      }

      return res;
    }

    template <class FBCommandT>
    std::unique_ptr<CommandUnion> encodeFlatbufferUnionT(const command::Command& c) {
      auto o = encodeFlatbufferUnionT(c.getObject()).release();
      auto fbCommand = std::make_unique<FBCommandT>();
      fbCommand->object = std::move(*o);
      auto res = std::make_unique<CommandUnion>();
      res->table = reinterpret_cast<flatbuffers::NativeTable *>(fbCommand.release());
      return res;
    }

    std::unique_ptr<CommandUnion> encodeFlatbufferUnionT(const command::Command& c) {

      const auto t = c.getCommandType();

      switch (t) {
        case command::CommandValueT::add: {
          return encodeFlatbufferUnionT<AddT>(c);
        }
        case command::CommandValueT::transfer: {
          return encodeFlatbufferUnionT<TransferT>(c);
        }
        case command::CommandValueT::update: {
          return encodeFlatbufferUnionT<UpdateT>(c);
        }
        case command::CommandValueT::remove: {
          return encodeFlatbufferUnionT<RemoveT>(c);
        }
        case command::CommandValueT::contract: {
          return encodeFlatbufferUnionT<ContractT>(c);
        }
        case command::CommandValueT::batch: {
          throw exception::NotImplementedException(
            "CommandValueT::batch is not implemented!",__FILE__
          );
//          return encodeFlatbufferUnionT<BatchT>(c);
        }
        case command::CommandValueT::unbatch: {
          throw exception::NotImplementedException(
            "CommandValueT::unbatch is not implemented!",__FILE__
          );
//          return encodeFlatbufferUnionT<UnbatchT>(c);
        }
        default: {
          std::cout << "command: "<< command::EnumNamesCommandValue(c.getCommandType()) << " ... " << std::endl;
          throw exception::NotImplementedException(
            "This command is not implemented!",__FILE__
          );
        }
      }
    }

    std::unique_ptr<iroha::TransactionT> encodeFlatbufferT(const event::Transaction& tx){
      std::cout << "\033[95m Tx event::Transaction \033[0m "<< tx.senderPublicKey << std::endl;
      std::unique_ptr<iroha::TransactionT> res(new iroha::TransactionT());

      res->sender = tx.senderPublicKey;
      res->hash   = tx.hash;

      std::vector<std::unique_ptr<TxSignatureT>> tsTv;
      for (const auto& t: tx.txSignatures()){
        auto ts = std::make_unique<TxSignatureT>();
        ts->publicKey = std::move(t.publicKey);
        ts->signature = std::move(t.signature);
        res->txSignatures.push_back( std::move(ts) );
        std::cout << "loop txSignatures\n";
      }

      {
        CommandUnion* command = encodeFlatbufferUnionT(tx.command).release();
        res->command = std::move(*command);
      }

      if (res->command.AsAdd() != nullptr) {
        if (res->command.AsAdd()->object.AsAsset() != nullptr) {
          std::cout << res->command.AsAdd()->object.AsAsset()->name << std::endl;
        }
      }

      return res;
    }

    std::unique_ptr<iroha::ConsensusEventT> encodeFlatbufferT(const event::ConsensusEvent& event){
        std::unique_ptr<iroha::ConsensusEventT> res(new iroha::ConsensusEventT());

        std::cout <<"consensus command ";

        for(const auto& tx: event.transactions){
          std::cout << "\033[95m Tx move \033[0m "<< tx.senderPublicKey << std::endl;
          res->transaction.emplace_back(encodeFlatbufferT(tx));
        }

        std::cout <<"event.transactions end\n";
        std::cout <<"event.eventSignatures start\n";

        for(auto&& e: event.eventSignatures()){
          std::unique_ptr<EventSignatureT> es(new EventSignatureT());
          es->publicKey = e.publicKey;
          es->signature = e.signature;
          res->eventSignatures.push_back( std::move(es) );
        }
        std::cout <<"event.eventSignatures end\n";
        std::cout <<"moved\n";
        res->state = State::State_Undetermined;
        return res;
    }


    class IrohaServiceImpl final : public Sumeragi::Service {
        flatbuffers::FlatBufferBuilder fbb_;
    public:

      ::grpc::Status Torii(::grpc::ClientContext* context, const flatbuffers::BufferRef<Request>& request, flatbuffers::BufferRef<Response>* response){
          // ToDo convertor
          fbb_.Clear();
          auto stat_offset = CreateResponse(fbb_,
              200,
              fbb_.CreateString("Hello, Ok")
          );
          fbb_.Finish(stat_offset);
          *response = flatbuffers::BufferRef<Response>(
              fbb_.GetBufferPointer(),
              fbb_.GetSize()
          );
          return grpc::Status::OK;
      }

      ::grpc::Status Verify(::grpc::ClientContext* context, const flatbuffers::BufferRef<iroha::ConsensusEvent>& request, flatbuffers::BufferRef<Response>* response){

        //ConsensusEventT *UnPack(const flatbuffers::resolver_function_t *_resolver = nullptr) const;
        //void UnPackTo(ConsensusEventT *_o, const flatbuffers::resolver_function_t *_resolver = nullptr) const;

        auto event = decodeFlatbufferT(*request.GetRoot()->UnPack());

        fbb_.Clear();
        auto stat_offset = CreateResponse(fbb_,
            200,
            fbb_.CreateString("Hello, Ok")
        );
        fbb_.Finish(stat_offset);
        *response = flatbuffers::BufferRef<Response>(fbb_.GetBufferPointer(),
                                                 fbb_.GetSize());
        return grpc::Status::OK;
      }

    };

    grpc::Server *server_instance = nullptr;
    std::mutex wait_for_server;
    std::condition_variable server_instance_cv;
    IrohaServiceImpl service;
    grpc::ServerBuilder builder;

    void initialize_peer() {
        std::string server_address("0.0.0.0:50051");
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
    }

    void finish(){
      if(server_instance != nullptr){
        server_instance->Shutdown();
        delete server_instance;
      }
    }

    bool send(
        const std::string& ip,
        const event::ConsensusEvent& event
    ) {
        logger::info("connection") << "start send";
        if (find(receiver_ips.begin(), receiver_ips.end(), ip) != receiver_ips.end()){

            logger::info("connection")  <<  "create client";
            auto channel = grpc::CreateChannel(
              "localhost:50051",
              grpc::InsecureChannelCredentials()
            );

            std::cout <<"send consensus command "<< \
              command::EnumNamesCommandValue(event.transactions.at(0).command.getCommandType()) << " ... "<< std::endl;


            auto stub = Sumeragi::NewStub(channel);
            grpc::ClientContext context;
            iroha::ConsensusEventT eventT;
            flatbuffers::FlatBufferBuilder fbb;
            fbb.Finish(iroha::ConsensusEvent::Pack( fbb, encodeFlatbufferT(event).get()));
            auto request = flatbuffers::BufferRef<iroha::ConsensusEvent>(
              fbb.GetBufferPointer(),
              fbb.GetSize()
            );

            flatbuffers::BufferRef<Response> response;

            auto status = stub->Verify(&context, request, &response);

            if (status.ok()) {
              auto message = response.GetRoot()->message();
              logger::info("connection sumeragi's response: ") << message->str();
            } else {
              logger::error("connection sumeragi response error");
            }
            return true;
        }else{
            logger::error("connection") <<  "not found";
            return false;
        }
    }

    bool sendAll(
        const event::ConsensusEvent& event
    ) {
        for (auto& ip : receiver_ips){
            if (ip != config::PeerServiceConfig::getInstance().getMyIp()){
                send( ip, event);
            }
        }
        return true;
    }

    bool receive(
      const std::function<void(
        const std::string&,
        event::ConsensusEvent&
      )>& callback) {
        receivers.puf_back(std::move(callback));
        return true;
    }

    void addSubscriber(std::string ip) {
        receiver_ips.push_back(ip);
    }

    int run() {
      wait_for_server.lock();
      server_instance = builder.BuildAndStart().release();
      wait_for_server.unlock();
      server_instance_cv.notify_one();

      logger::info("Torii runing!");
      server_instance->Wait();
      return 0;
    }
}
