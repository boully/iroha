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

#include <thread>

#include <grpc++/grpc++.h>

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

    namespace convertor {

      std::pair<std::string, object::BaseObject> decodeObject(iroha::BaseObjectT& obj) {
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

      object::Object decodeObject(ObjectUnion& u) {
        switch (u.type) {
          case Object::Object_Asset:{
            auto asset = object::Asset();
            asset.domain = u.AsAsset()->domain;
            asset.name   = u.AsAsset()->name;
            for (std::size_t i = 0; i < u.AsAsset()->objects.size(); i++) {
              auto o = *u.AsAsset()->objects[i];
              asset.value.insert(decodeObject(o));
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

      command::Command decodeCommand(CommandUnion& u) {
        switch(u.type){
          case Command::Command_Add:{
            auto object = decodeObject(u.AsAdd()->object);
            return command::Command(command::Add(object));
          }
          case Command::Command_Transfer:{
            auto object = decodeObject(u.AsTransfer()->object);
            return command::Command(command::Transfer(object,u.AsTransfer()->receiver));
          }
          case Command::Command_Contract:{
            auto object = decodeObject(u.AsContract()->object);
            return command::Command(command::Contract(object,u.AsContract()->contractName));
          }
          case Command::Command_Remove:{
            auto object = decodeObject(u.AsRemove()->object);
            return command::Command(command::Remove(object));
          }
          case Command::Command_Batch:{
            throw exception::NotImplementedException("decodeCommand: Command_Batch", __FILE__);
//            return command::Command(command::Batch());
          }
          case Command::Command_Unbatch:{
            throw exception::NotImplementedException("decodeCommand: Command_Unbatch", __FILE__);
//            return command::Command(command::Unbatch());
          }
          case Command::Command_Update:{
            auto object = decodeObject(u.AsUpdate()->object);
            return command::Command(command::Update(object));
          }
          default:{
          }
        }
      }

      event::Transaction decodeTransaction(TransactionT& u) {
        event::Transaction res;
        std::cout << "NOT IMPLEMENTED decodeTransaction(TransactionT)\n";
        return res;
      }

      event::ConsensusEvent decodeConsensusEvent(ConsensusEventT& u) {
        if(u.transaction.empty()){
          throw exception::NotImplementedException(
            "Multi transaction supporting is not implemented.",__FILE__
          );
        }
        event::ConsensusEvent res( std::move(decodeTransaction(*u.transaction.at(0))) );
        for(auto&& esig: u.eventSignatures){
          res.addEventSignature(std::move(esig->publicKey),std::move(esig->signature));
        }
        return res;
      }

      std::unique_ptr<iroha::BaseObjectT> encodeBaseObject(const std::string& name, const object::BaseObject& obj){
        std::unique_ptr<iroha::BaseObjectT> res(new iroha::BaseObjectT());

        res->name = name;
        res->text = "";res->integer = 0;res->boolean = false;res->decimal = 0.0f;

        if(obj.object_type == object::BaseObject::Object_type::STRING){
          res->type     = BaseObjectType::BaseObjectType_Text;
          res->text     = (std::string)obj;
          std::cout << "(std::string)obj" << (std::string)obj << ", " << res->text << std::endl;
        }else if(obj.object_type == object::BaseObject::Object_type::INTEGER){
          res->type     = BaseObjectType::BaseObjectType_Integer;
          res->integer  = (int)obj;
          std::cout << "(int)obj" << (int)obj << ", " << res->integer << std::endl;
        }else if(obj.object_type == object::BaseObject::Object_type::BOOLEAN){
          res->type     = BaseObjectType::BaseObjectType_Boolean;
          res->boolean  = (bool)obj;
          std::cout << "(bool)obj" << (bool)obj << ", " << res->boolean << std::endl;
        }else if(obj.object_type == object::BaseObject::Object_type::FLOAT){
          res->type     = BaseObjectType::BaseObjectType_Decimal;
          res->decimal  = (float)obj;
          std::cout << "(float)obj" << (float)obj << ", " << res->decimal << std::endl;
        }else{
          throw exception::NotImplementedException(
            "This base object type is not implemented.",__FILE__
          );
        }
        return res;
      }

      std::unique_ptr<ObjectUnion> encodeObject(const object::Object& obj) {
        if(obj.type == object::ObjectValueT::asset){
          auto assetT = AssetT();
          assetT.domain = obj.asset->domain;
          assetT.name   = obj.asset->name;
          std::cout << "----------------------------------------------------------------------------\n";
          std::cout << "assetT.domain = " << assetT.domain  << std::endl;
          std::cout << "assetT.name = "   << assetT.name    << std::endl;
          for (const auto& o: obj.asset->value) {

            // Verify valid object.
            {
              std::cout << o.first << std::endl;
              if(o.second.object_type == object::BaseObject::Object_type::STRING){
                std::cout << (std::string)o.second << std::endl;
              }else if(o.second.object_type == object::BaseObject::Object_type::INTEGER){
                std::cout << (int)o.second << std::endl;
              }else if(o.second.object_type == object::BaseObject::Object_type::BOOLEAN){
                std::cout << (bool)o.second << std::endl;
              }else if(o.second.object_type == object::BaseObject::Object_type::FLOAT){
                std::cout << (float)o.second << std::endl;
              }else{
                throw exception::NotImplementedException(
                  "This base object type is not implemented.",__FILE__
                );
              }
            }

            // Pack.
            assetT.objects.emplace_back(encodeBaseObject(o.first, o.second));

            // Verify successful packing.
            std::cout << "--------------- BEGIN VERIFY encoded ObjectT ---------------\n";
            {
              for (const auto& e: assetT.objects) {
                if(e->type == BaseObjectType::BaseObjectType_Text){
                  std::cout << e->text << std::endl;
                }else if(e->type == BaseObjectType::BaseObjectType_Integer){
                  std::cout << e->integer << std::endl;
                }else if(e->type == BaseObjectType::BaseObjectType_Boolean){
                  std::cout << e->boolean << std::endl;
                }else if(e->type == BaseObjectType::BaseObjectType_Decimal){
                  std::cout << e->decimal << std::endl;
                }else{
                  throw exception::NotImplementedException(
                    "This base object type is not implemented.",__FILE__
                  );
                }
              }
            }
            std::cout << "--------------- END VERIFY encoded ObjectT ---------------\n";


          }

          return std::make_unique<ObjectUnion>(std::move(assetT));
        }
  /*
        else if(obj.type == object::ObjectValueT::domain){
          auto domainT = DomainT();
          throw exception::NotImplementedException(
            "This domain is not implemented.",__FILE__
          );
          res->Set(std::move(domainT));
        }else if(obj.type == object::ObjectValueT::account){
          auto accountT = AccountT();
          throw exception::NotImplementedException(
            "This domain is not implemented.",__FILE__
          );
          res->Set(std::move(accountT));
        }else if(obj.type == object::ObjectValueT::peer){
          auto peerT = PeerT();
          throw exception::NotImplementedException(
            "This domain is not implemented.",__FILE__
          );
          res->Set(std::move(peerT));
        }else{
          throw exception::NotImplementedException(
            "This object is not implemented.",__FILE__
          );
        }
  */
      }

      std::unique_ptr<CommandUnion> encodeCommand(const command::Command& command) {
        if(command.getCommandType() == command::CommandValueT::add){

            // Pack
            auto addT = AddT();
            auto objectUnionPtr = encodeObject(command.getObject()).release(); // UnionObjectはenumの型typeが定まり、実態がある状態
            addT.object = ObjectUnion(std::move(*objectUnionPtr));

            // Verify
            std::cout << "--------------- BEGIN VERIFY encodeCommand ---------------\n";
            {
              auto const& e = addT.object;
              if(e.type == Object::Object_Asset){
                auto p = reinterpret_cast<AssetT *>(e.table);
                std::cout << p->domain << std::endl;
                std::cout << p->name << std::endl;
              }else if(e.type == Object::Object_Domain){
                auto p = reinterpret_cast<DomainT *>(e.table);
              }else if(e.type == Object::Object_Account){
                auto p = reinterpret_cast<AccountT *>(e.table);
              }else if(e.type == Object::Object_Peer){
                auto p = reinterpret_cast<PeerT *>(e.table);
              }else{
                throw exception::NotImplementedException(
                  "This base object type is not implemented.",__FILE__
                );
              }
            }
            std::cout << "--------------- END VERIFY encodeCommand ---------------\n";

            return std::make_unique<CommandUnion>(std::move(addT));
        }
  /*
        }else if(command.getCommandType() == command::CommandValueT::transfer){
            auto transferT = std::make_unique<TransferT>();
            ObjectUnion* obj = encodeFlatbufferUnionT(command.getObject()).release();
            transferT->object = std::move(*obj);
            res->Set(std::move(*transferT));
        }else if(command.getCommandType() == command::CommandValueT::update){
            auto updateT = UpdateT();
            ObjectUnion* obj = encodeFlatbufferUnionT(command.getObject()).release();
            updateT->object = std::move(*obj);
            res->Set(std::move(updateT));
        }else if(command.getCommandType() == command::CommandValueT::remove){
          auto removeT = RemoveT();
          ObjectUnion* obj = encodeFlatbufferUnionT(command.getObject()).release();
          removeT->object = std::move(*obj);
          res->Set(std::move(removeT));
        }else if(command.getCommandType() == command::CommandValueT::contract){
          auto contractT = ContractT();
          ObjectUnion obj = encodeFlatbufferUnionT(command.getObject());
          contractT->object = std::move(*obj);
          res->Set(std::move(contractT));
        }else if(command.getCommandType() == command::CommandValueT::batch){
          auto batchT = BatchT();
          res->Set(std::move(batchT));
        }else if(command.getCommandType() == command::CommandValueT::unbatch){
          auto unbatchT = UnbatchT();
          res->Set(std::move(unbatchT));
        }else{
          std::cout <<"comd "<< command::EnumNamesCommandValue(command.getCommandType()) << " ... "<< std::endl;
          throw exception::NotImplementedException(
            "This command is not implemented!",__FILE__
          );
        }
  */
      }

      std::unique_ptr<iroha::TransactionT> encodeTransaction(const event::Transaction& tx) {
        std::cout << "\033[95m Tx event::Transaction \033[0m "<< tx.senderPublicKey << std::endl;
        std::unique_ptr<iroha::TransactionT> res(new iroha::TransactionT());  // HogeT does not have ctor(T&&)
        res->sender = tx.senderPublicKey;
        res->hash   = tx.hash;

        std::vector<std::unique_ptr<TxSignatureT>> tsTv;
        for(const auto& t: tx.txSignatures()){
          auto ts = std::make_unique<TxSignatureT>();
          ts->publicKey = t.publicKey;
          ts->signature = t.signature;
          res->txSignatures.push_back( std::move(ts) );
          std::cout << "res->txSignatures.back()->publicKey = " << res->txSignatures.back()->publicKey << "\n";
          std::cout << "loop txSignatures\n";
        }

        auto commandUnionPtr = encodeCommand(tx.command).release();
        res->command = std::move(*commandUnionPtr);

        // Verify
        std::cout << "--------------- BEGIN VERIFY encodeTransaction ---------------\n";
        {
          auto const& e = res->command;
          if(e.type == Command::Command_Add){
            auto p = reinterpret_cast<AddT *>(e.table);


            std::cout << "--------------- BEGIN VERIFY union in union ---------------\n";
            {
              auto const& e = p->object;
              if(e.type == Object::Object_Asset){
                auto p = reinterpret_cast<AssetT *>(e.table);
                std::cout << p->domain << std::endl;
                std::cout << p->name << std::endl;
              }else if(e.type == Object::Object_Domain){
                auto p = reinterpret_cast<DomainT *>(e.table);
              }else if(e.type == Object::Object_Account){
                auto p = reinterpret_cast<AccountT *>(e.table);
              }else if(e.type == Object::Object_Peer){
                auto p = reinterpret_cast<PeerT *>(e.table);
              }else{
                throw exception::NotImplementedException(
                  "This base object type is not implemented.",__FILE__
                );
              }
            }
            std::cout << "--------------- END VERIFY union in union ---------------\n";

          }else if(e.type == Command::Command_Transfer){
            auto p = reinterpret_cast<TransferT *>(e.table);
          }else if(e.type == Command::Command_Update){
            auto p = reinterpret_cast<UpdateT *>(e.table);
          }else if(e.type == Command::Command_Remove){
            auto p = reinterpret_cast<RemoveT *>(e.table);
          }else{
            throw exception::NotImplementedException(
              "This base object type is not implemented.",__FILE__
            );
          }
        }
        std::cout << "--------------- END VERIFY encodeTransaction ---------------\n";

        return res;
      }

      std::unique_ptr<iroha::ConsensusEventT> encodeConsensusEvent(const event::ConsensusEvent& event){
          std::unique_ptr<iroha::ConsensusEventT> res(new iroha::ConsensusEventT());

          std::cout <<"consensus command ";
          for(const auto& tx: event.transactions){
            std::cout << "\033[95m Tx move \033[0m "<< tx.senderPublicKey << std::endl;
            res->transaction.emplace_back(encodeTransaction(tx));
          }
          std::cout <<"event.transactions end\n";

          std::cout <<"event.eventSignatures start\n";
          for(const auto& e: event.eventSignatures()){
            std::unique_ptr<EventSignatureT> es(new EventSignatureT());
            es->publicKey = e.publicKey;
            es->signature = e.signature;
            res->eventSignatures.emplace_back( std::move(es) );
          }
          std::cout <<"event.eventSignatures end\n";
          std::cout <<"moved\n";
          res->state = State::State_Undetermined;
          return std::move(res);
      }
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
        std::cout<< "Recv \n";
        auto event = convertor::decodeConsensusEvent(*request.GetRoot()->UnPack());

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
            std::cout << "ININININININININ\n";
            auto uptr = convertor::encodeConsensusEvent(event);
            /*
            auto eventSignatures = uptr->eventSignatures;
            for (auto&& e: eventSignatures) {

            }
            */
            fbb.Finish(iroha::ConsensusEvent::Pack( fbb, uptr.release()));
            std::cout << "OUTOUOTUOUTOUTOUTOUTUOT\n";
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
        receivers.push_back(std::move(callback));
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
