#include <WebsocketService.hpp>
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#include <iostream>

using namespace std;

WebsocketService::WebsocketService(char *url)
{
    this->url = url;
    cout << "WebsocketService::WebsocketService() unimplemented" << endl;
}

WebsocketService::~WebsocketService()
{
    cout << "WebsocketService::~WebsocketService() unimplemented" << endl;
}

void WebsocketService::register_onopen_callback(std::function<void()> callback)
{
    onopen_callback = callback;
    cout << "Registed a callback on socket open." << endl;
}

void WebsocketService::register_onerror_callback(std::function<void(int, void *)> callback)
{
    cout << "WebsocketService::register_onerror_callback() unimplemented" << endl;
}

void WebsocketService::register_onclose_callback(std::function<void(int, void *)> callback)
{
    cout << "WebsocketService::register_onclose_callback() unimplemented" << endl;
}

void WebsocketService::register_onmessage_callback(std::function<void(char* message)> callback)
{
    onmessage_callback = callback;
}

void WebsocketService::send_utf8_text(const char *message)
{
    cout << "Attempting to send " << message << " to " << currentWS << endl;
    EMSCRIPTEN_RESULT result = emscripten_websocket_send_utf8_text(currentWS, message);
    if (result)
    {
        printf("Failed to emscripten_websocket_send_utf8_text(): %d\n", result);
    }
}

void WebsocketService::send_binary(const char *message, int length)
{
    cout << "WebsocketService::send_binary() unimplemented" << endl;
}

void WebsocketService::close(int code, const char *reason)
{
    cout << "WebsocketService::close() unimplemented" << endl;
}

EM_BOOL WebsocketService::onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData)
{

    puts("onopen");
    ((WebsocketService *)userData)->connected = true;

    EMSCRIPTEN_RESULT result;
    result = emscripten_websocket_send_utf8_text(websocketEvent->socket, "hello world");
    if (result)
    {

        printf("Failed to emscripten_websocket_send_utf8_text(): %d\n", result);
    }

    if (((WebsocketService *)userData)->onopen_callback)
    {
        ((WebsocketService *)userData)->onopen_callback();
    }

    return EM_TRUE;
}
EM_BOOL WebsocketService::onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData)
{

    puts("onerror");

    return EM_TRUE;
}
EM_BOOL WebsocketService::onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData)
{

    puts("onclose");

    return EM_TRUE;
}

EM_BOOL WebsocketService::onmessage(int eventType,
                                    const EmscriptenWebSocketMessageEvent *websocketEvent,
                                    void *userData)
{

    puts("onmessage");

    if (websocketEvent->isText)
    {
        // For only ascii chars.
        printf("message: %s\n", websocketEvent->data);
        // Convert from uint8_t to char
        char *message = (char *)websocketEvent->data;

        if (((WebsocketService *)userData)->onmessage_callback)
        {
            ((WebsocketService *)userData)->onmessage_callback(message);
        }
    };


    return EM_TRUE;
}

void WebsocketService::init()
{

    EmscriptenWebSocketCreateAttributes ws_attrs = {
        this->url,
        NULL,
        EM_TRUE};

    EMSCRIPTEN_WEBSOCKET_T ws = emscripten_websocket_new(&ws_attrs);
    this->currentWS = ws;
    void *context = (void *)this;
    emscripten_websocket_set_onopen_callback(ws, context, this->onopen);
    emscripten_websocket_set_onerror_callback(ws, context, this->onerror);
    emscripten_websocket_set_onclose_callback(ws, context, this->onclose);
    emscripten_websocket_set_onmessage_callback(ws, context, this->onmessage);

    // Synchronously wait until connection has been established.
    uint16_t readyState = 0;
    do {
        emscripten_websocket_get_ready_state(ws, &readyState);
        emscripten_sleep(100);
    } while(readyState == 0);

}

bool WebsocketService::is_connected()
{
    return connected;
}