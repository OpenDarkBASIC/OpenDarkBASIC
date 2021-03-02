/**
 * This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!
 */

#ifndef JSONRPC_CPP_STUB_ABSTRACTSTUBSERVER_H_
#define JSONRPC_CPP_STUB_ABSTRACTSTUBSERVER_H_

#include <jsonrpccpp/server.h>

class AbstractStubServer : public jsonrpc::AbstractServer<AbstractStubServer>
{
    public:
        AbstractStubServer(jsonrpc::AbstractServerConnector &conn, jsonrpc::serverVersion_t type = jsonrpc::JSONRPC_SERVER_V2) : jsonrpc::AbstractServer<AbstractStubServer>(conn, type)
        {
            this->bindAndAddMethod(jsonrpc::Procedure("sayHello", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, "name",jsonrpc::JSON_STRING, NULL), &AbstractStubServer::sayHelloI);
            this->bindAndAddNotification(jsonrpc::Procedure("notifyServer", jsonrpc::PARAMS_BY_NAME,  NULL), &AbstractStubServer::notifyServerI);
        }

        inline virtual void sayHelloI(const Json::Value &request, Json::Value &response)
        {
            response = this->sayHello(request["name"].asString());
        }
        inline virtual void notifyServerI(const Json::Value &/*request*/)
        {
            this->notifyServer();
        }
        virtual std::string sayHello(const std::string& name) = 0;
        virtual void notifyServer() = 0;
};

#endif //JSONRPC_CPP_STUB_ABSTRACTSTUBSERVER_H_