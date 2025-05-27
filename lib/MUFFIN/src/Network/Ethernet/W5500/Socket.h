/**
 * @file Sockets.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Represents a network socket for the W5500 Ethernet interface.
 * 
 * This class provides an abstraction for managing network sockets on the W5500
 * Ethernet interface. It supports operations such as opening, closing, binding,
 * connecting, sending, and receiving data over TCP or UDP protocols.
 * 
 * @note The class is tightly coupled with the W5500 Ethernet interface and
 *       requires a reference to the interface object during construction.
 * 
 * @details
 * - The socket is identified by an index (`sock_id_e`) and operates using a
 *   specified protocol (`sock_prtcl_e`).
 * - Provides methods for both connection-oriented (TCP) and connectionless (UDP)
 *   communication.
 * - Includes utility functions for handling interrupts and managing socket states.
 * 
 * @section Usage
 * 1. Create an instance of the `Socket` class by providing a reference to the
 *    W5500 interface, socket ID, and protocol type.
 * 2. Use the `Open` method to initialize the socket.
 * 3. Perform operations such as `Bind`, `Connect`, `Send`, or `Receive` as needed.
 * 4. Close the socket using the `Close` method when done.
 * 
 * @section Example
 * @code
 * W5500 ethernetInterface;
 * Socket socket(ethernetInterface, SOCK_ID_0, PROTOCOL_TCP);
 * 
 * if (socket.Open() == Status::SUCCESS) {
 *     socket.Bind(IPAddress(192, 168, 1, 100), 8080);
 *     socket.Listen();
 *     // Handle incoming connections or data
 *     socket.Close();
 * }
 * @endcode
 * 
 * @section Limitations
 * - The maximum number of retries for certain operations is limited to `MAX_TRIAL_COUNT`.
 * - The class assumes that the W5500 interface is properly initialized and configured.
 * 
 * @date 2025-05-08
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */
#if defined(MT11)



#pragma once

#include <esp32-hal.h>
#include <Arduino.h>

#include "Common/Status.h"
#include "Common/Logger/Logger.h"
#include "Include/TypeDefinitions.h"
#include "Include/Converter.h"
#include "W5500.h"

// #include "wizchip_conf.h"
constexpr uint8_t MAX_TRIAL_COUNT = 5;


namespace muffin { namespace w5500 {


    typedef enum class SocketErrorCodeEnum
    {
        OK                    =  1,
        BUSY                  =  0,
        ERRROR                = -1,
        ERROR_ID              = ERRROR -  1,
        ERROR_OPTION          = ERRROR -  2,
        ERROR_INIT            = ERRROR -  3,
        ERROR_CLOSED          = ERRROR -  4,
        ERROR_MODE            = ERRROR -  5,
        ERROR_FLAG            = ERRROR -  6,
        ERROR_STATUS          = ERRROR -  7,
        ERROR_ARGUMENT        = ERRROR - 10,
        ERROR_PORT_ZERO       = ERRROR - 11,
        ERROR_INVALID_IP      = ERRROR - 12,
        ERROR_TIMEOUT         = ERRROR - 13,
        ERROR_DATA_LENGTH     = ERRROR - 14,
        ERROR_BUFFER          = ERRROR - 15,
        FATAL                 = -1000,
        FATAL_PACKET_LENGTH   = FATAL - 1,
    } sock_err_e;

    
    class Socket
    {
        friend class EthernetClient;
        
    public:
        /**
         * @brief Construct a network socket for the W5500 Ethernet interface.
         * 
         * @param interface Reference to the W5500 Ethernet interface object.
         * @param idx The socket identifier (index) to be used.
         * @param protocol The protocol type (e.g., TCP, UDP) for the socket.
         */
        Socket(W5500& interface, const sock_id_e idx, const sock_prtcl_e protocol);

        /**
         * @brief Destructor for the Socket class.
         * 
         * Cleans up resources associated with the Socket instance.
         */
        ~Socket();
    public:
        /**
         * @brief Retrieves the current status of the socket.
         * 
         * @return ssr_e The status of the socket as an enumerated value.
         */
        ssr_e GetStatus();
        Status GetInterrupt(sir_t* interrupt);
        sock_id_e GetSocketID() const;
        sock_prtcl_e GetProtocol() const;
        int Available();


    public:
        /**
         * @brief Opens a network socket for communication.
         * 
         * This function initializes and configures the socket for use with the W5500 Ethernet module.
         * It prepares the socket for sending and receiving data over the network.
         * 
         * @return Status Returns the status of the operation, indicating success or failure.
         */
        Status Open();


        /**
         * @brief Closes the socket and releases any associated resources.
         * 
         * This function terminates the socket connection and ensures that
         * all resources allocated for the socket are properly released.
         * After calling this function, the socket will no longer be usable
         * until it is reinitialized.
         * 
         * @return Status Returns the status of the operation, indicating
         *         success or failure.
         */
        Status Close();


        /**
         * @brief Binds the socket to a specific port number.
         * 
         * This function assigns the socket to the specified port, allowing it to 
         * listen for incoming connections or send data from that port.
         * 
         * @param port The port number to bind the socket to. Must be a valid 
         *             16-bit unsigned integer.
         * @return Status Returns a status indicating the success or failure of 
         *         the bind operation.
         */
        Status Bind(const uint16_t port);


        /**
         * @brief Listens for incoming connections on the socket.
         * 
         * This function prepares the socket to accept incoming connections
         * from remote hosts. It sets the socket to a listening state, allowing
         * it to accept connection requests.
         * 
         * @return Status The status of the listen operation.
         */
        Status Listen();


        /**
         * @brief Accepts a connection from a remote host.
         * 
         * This function waits for an incoming connection request and establishes
         * a connection with the remote host. It retrieves the IP address and port
         * of the connected host.
         * 
         * @param ip Pointer to an IPAddress object to store the connected host's IP address.
         * @param port Pointer to a variable to store the connected host's port number.
         * @return Status The status of the accept operation.
         */
        Status Accept(IPAddress* ip, uint16_t* port);


        /**
         * @brief Establishes a connection to a remote host using the specified IP address and port.
         * 
         * @param ip The IP address of the remote host to connect to.
         * @param port The port number on the remote host to connect to.
         * @return Status The status of the connection attempt.
         */
        Status Connect(const IPAddress ip, const uint16_t port);


        /**
         * @brief Terminates the connection for the socket.
         * 
         * This function is used to disconnect an established connection
         * on the socket. It ensures that the socket is properly closed
         * and any associated resources are released.
         * 
         * @return Status Returns the status of the disconnect operation.
         *         Possible values include success or an error code indicating
         *         the reason for failure.
         */
        Status Disconnect();

        
    public:
        /**
         * @brief Sends data through the socket.
         * 
         * This function transmits a specified amount of data through the socket.
         * 
         * @param length The number of bytes to send.
         * @param data A pointer to the array of bytes to be sent.
         * @return Status The status of the send operation.
         */
        Status Send(const size_t length, const uint8_t data[]);
        

        /**
         * @brief Receives data from the socket.
         * 
         * @param length The maximum number of bytes to receive.
         * @param actualLength Pointer to a variable where the actual number of bytes received will be stored.
         * @param data Pointer to a buffer where the received data will be stored.
         * @return Status indicating the result of the receive operation.
         */
        Status Receive(const size_t length, size_t* actualLength, uint8_t* data);


        /**
         * @brief Sends data to a specified IP address and port using the socket.
         * 
         * @param ip The destination IP address to send the data to.
         * @param port The destination port to send the data to.
         * @param length The length of the data to be sent.
         * @param data A pointer to the array of data to be sent.
         * @return Status The status of the send operation, indicating success or failure.
         */
        Status SendTo(const IPAddress ip, const uint16_t port, const size_t length, const uint8_t data[]);


        /**
         * @brief Receives data from a specific IP address and port.
         * 
         * @param ip The IP address of the sender.
         * @param port The port number of the sender.
         * @param length The maximum length of data to receive.
         * @param actualLength Pointer to a variable where the actual length of received data will be stored.
         * @param data Pointer to a buffer where the received data will be stored.
         * @return Status The status of the receive operation.
         */
        Status ReceiveFrom(const IPAddress ip, const uint16_t port, const size_t length, size_t* actualLength, uint8_t* data);


    private:
        /**
         * @brief Waits until the command register of the W5500 Ethernet controller is cleared.
         * 
         * This function ensures that the previous command issued to the W5500 has been
         * processed and the command register is ready for the next operation. It is
         * typically used to synchronize operations with the W5500 to avoid conflicts
         * or unexpected behavior.
         */
        void waitCommandCleared();


        /**
         * @brief Waits for an interrupt event to occur on the socket.
         * 
         * This function blocks execution until a specific interrupt event is detected,
         * allowing the program to respond to hardware-level events on the W5500 Ethernet module.
         * 
         * @note Ensure proper interrupt configuration before calling this function.
         */
        void waitInterruptEvnet();


        
        /**
         * @brief Clears the specified socket interrupt.
         *
         * This function clears the interrupt flag for the given interrupt type
         * associated with the socket. It is typically used to acknowledge and
         * reset interrupt conditions after they have been handled.
         *
         * @param interrupt The interrupt type to clear, specified as a value of the sir_e enumeration.
         */
        void clearInterrupt(const sir_e interrupt);
        
        
        /**
         * @brief Clears the interrupt flags for the socket.
         * 
         * This function is used to reset or clear any interrupt flags
         * associated with the socket, ensuring that the socket is in
         * a clean state for further operations.
         */
        void clearInterruptAll();
        
        
        /**
         * @brief Sets the IPv4 address of the host for the socket.
         * 
         * This function configures the IPv4 address of the host that the socket
         * will use for communication. The provided IP address must be in the 
         * format of an IPAddress object.
         * 
         * @param ip The IPv4 address to set, represented as an IPAddress object.
         * @return Status Returns a status indicating the success or failure of the operation.
         */
        Status setHostIPv4(const IPAddress ip);
        
        
        /**
         * @brief Sets the host port for the socket.
         * 
         * This function configures the port number that the socket will use
         * for communication. The port number must be within the valid range
         * for network communication.
         * 
         * @param port The port number to set for the socket.
         *             Must be a 16-bit unsigned integer.
         * 
         * @return Status indicating the success or failure of the operation.
         */
        Status setHostPort(const uint16_t port);


        /**
         * @brief Sets the local port for the socket.
         * 
         * This function configures the local port number that the socket
         * will use for communication. The local port is typically used for
         * binding the socket to a specific port on the local device.
         * 
         * @param port The local port number to set for the socket.
         *             Must be a 16-bit unsigned integer.
         * 
         * @return Status indicating the success or failure of the operation.
         */
        Status setSourcePort(const uint16_t port);
        
        
        /**
         * @brief Retrieves the available free size in the transmit buffer of the socket.
         * 
         * This function checks the current state of the transmit buffer and returns
         * the amount of free space available for sending data. It is useful for ensuring
         * that there is enough space in the buffer before attempting to send data.
         * 
         * @return Status indicating the success or failure of the operation.
         */
        Status retrieveTxFreeSize();
        
        
        /**
         * @brief Retrieves the size of the data received in the socket's receive buffer.
         * 
         * This function checks the socket's receive buffer and determines the amount
         * of data currently available to be read. It is useful for managing incoming
         * data and ensuring that the correct amount of data is processed.
         * 
         * @return Status indicating the success or failure of the operation.
         */
        Status retrieveReceivedSize();
        
        
        Status implementSend(const size_t length, const uint8_t data[]);
        
        
        Status implementReceive(const size_t length, uint8_t* data);
    private:
        static const uint8_t MAX_TRIAL_COUNT = 5;
    private:
        W5500& mW5500;
        w5500::srb_t mSRB;
        const sock_id_e mID;
        const sock_prtcl_e mProtocol;
    };
}}

#endif