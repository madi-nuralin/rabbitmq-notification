#ifndef MESSAGE_BROKER_H
#define MESSAGE_BROKER_H

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace gs {
namespace amqp {

/** basic class properties */
struct AmqpProperties
{
  std::optional<std::string> content_type;
  std::optional<std::string> content_encoding;
  std::optional<uint8_t> delivery_mode;
  std::optional<uint8_t> priority;
  std::optional<std::string> correlation_id;
  std::optional<std::string> reply_to;
  std::optional<std::string> expiration;
  std::optional<std::string> message_id;
  std::optional<uint64_t> timestamp;
  std::optional<std::string> type;
  std::optional<std::string> user_id;
  std::optional<std::string> app_id;
  std::optional<std::string> cluster_id;
};

class AmqpMessage
{
public:
  using Ptr = std::shared_ptr<AmqpMessage>;
  using WPtr = std::weak_ptr<AmqpMessage>;

  AmqpMessage();
  virtual ~AmqpMessage();

  /**
   * Gets the message body as a std::string&
   */
  const std::string& body() const noexcept;
  std::string& body() noexcept;

  /**
   * Gets the message properties as a AmqpProperties&
   */
  const AmqpProperties& properties() const noexcept;
  AmqpProperties& properties() noexcept;

  static Ptr createInstance() { return std::make_shared<AmqpMessage>(); }

private:
  struct Impl;
  /// PIMPL idiom
  std::unique_ptr<Impl> m_impl;
};

class AmqpEnvelope
{
public:
  using Ptr = std::shared_ptr<AmqpEnvelope>;
  using WPtr = std::weak_ptr<AmqpEnvelope>;

  AmqpEnvelope(const AmqpMessage::Ptr message,
               const std::string& consumer_tag,
               const std::uint64_t delivery_tag,
               const std::string& exchange,
               bool redelivered,
               const std::string& routing_key);
  virtual ~AmqpEnvelope();

  inline AmqpMessage::Ptr message() const { return m_message; }

  inline std::string consumerTag() const { return m_consumerTag; }

  inline std::uint64_t deliveryTag() const { return m_deliveryTag; }

  inline std::string exchange() const { return m_exchange; }

  inline bool redelivered() const { return m_redelivered; }

  inline std::string routingKey() const { return m_routingKey; }

  static Ptr createInstance(const AmqpMessage::Ptr message,
                            const std::string& consumer_tag,
                            const std::uint64_t delivery_tag,
                            const std::string& exchange,
                            bool redelivered,
                            const std::string& routing_key)
  {
    return std::make_shared<AmqpEnvelope>(
      message, consumer_tag, delivery_tag, exchange, redelivered, routing_key);
  }

private:
  AmqpEnvelope() = delete;

  const AmqpMessage::Ptr m_message;
  const std::string m_consumerTag;
  const std::uint64_t m_deliveryTag;
  const std::string m_exchange;
  const bool m_redelivered;
  const std::string m_routingKey;
};

class AmqpConnection
{
public:
  using Ptr = std::shared_ptr<AmqpConnection>;
  using WPtr = std::weak_ptr<AmqpConnection>;

  AmqpConnection();
  virtual ~AmqpConnection();

  void open(const std::string& host, int port);
  void login(const std::string& vhost,
             const std::string& username,
             const std::string& password,
             int frame_max) const;

  static Ptr createInstance() { return std::make_shared<AmqpConnection>(); }

  // private:
  struct Impl;
  /// PIMPL idiom
  std::unique_ptr<Impl> m_impl;
};

class AmqpChannel
{
  ///< `"direct"` string constant
  static const char* EXCHANGE_TYPE_DIRECT;

  ///< `"fanout"` string constant
  static const char* EXCHANGE_TYPE_FANOUT;

  ///< `"topic"` string constant
  static const char* EXCHANGE_TYPE_TOPIC;

public:
  using Ptr = std::shared_ptr<AmqpChannel>;
  using WPtr = std::weak_ptr<AmqpChannel>;

  AmqpChannel(const AmqpConnection::Ptr conn);
  virtual ~AmqpChannel();

  /**
   * Declares an exchange
   *
   * Creates an exchange on the AMQP broker if it does not already exist
   * @param exchange_name the name of the exchange
   * @param exchange_type the type of exchange to be declared. Defaults to
   * `direct`; other types that could be used: `fanout`, `topic`
   * @param passive Indicates how the broker should react if the exchange does
   * not exist. If passive is `true` and the exhange does not exist the broker
   * will respond with an error and not create the exchange; exchange is created
   * otherwise. Defaults to `false` (exchange is created if needed)
   * @param durable Indicates whether the exchange is durable - e.g., will it
   * survive a broker restart.
   * @param auto_delete Indicates whether the exchange will automatically be
   * removed when no queues are bound to it.
   */
  void exchangeDeclare(
    const std::string& exchange_name,
    const std::string& exchange_type = AmqpChannel::EXCHANGE_TYPE_DIRECT,
    bool passive = false,
    bool durable = false,
    bool auto_delete = false,
    bool internal = false);

  /**
   * Binds one exchange to another exchange using a given key
   * @param destination the name of the exchange to route messages to
   * @param source the name of the exchange to route messages from
   * @param routing_key the routing key to use when binding
   */
  void exchangeBind(const std::string& destination,
                    const std::string& source,
                    const std::string& routing_key);

  /**
   * Unbind an existing exchange-exchange binding
   * @see BindExchange
   * @param destination the name of the exchange to route messages to
   * @param source the name of the exchange to route messages from
   * @param routing_key the routing key to use when binding
   */
  void exchangeUnbind(const std::string& destination,
                      const std::string& source,
                      const std::string& routing_key);

  /**
   * Declare a queue
   *conn
   * Creates a queue on the AMQP broker if it does not already exist.
   * @param queue_name The desired name of the queue. If this is an empty
   * string, the broker will generate a queue name that this method will return.
   * @param passive Indicated how the broker should react if the queue does not
   * exist. The broker will raise an error if the queue doesn't already exist
   * and passive is `true`. With passive `false` (the default), the queue gets
   * created automatically, if needed.
   * @param durable Indicates whether the exchange is durable - e.g., will it
   * survive a broker restart.
   * @param exclusive Indicates that only client can use the queue. Defaults to
   * true. An exclusive queue is deleted when the connection is closed.
   * @param auto_delete the queue will be deleted after at least one exchange
   * has been bound to it, then has been unbound
   * @returns The name of the queue created on the broker. Used mostly when the
   * broker is asked to create a unique queue by not providing a queue name.
   */
  std::string queueDeclare(const std::string& queue_name,
                           bool passive = false,
                           bool durable = false,
                           bool exclusive = true,
                           bool auto_delete = true);

  /**
   * Binds a queue to an exchange
   *
   * Connects a queue to an exchange on the broker.
   * @param queue_name The name of the queue to bind.
   * @param exchange_name The name of the exchange to bind to.
   * @param routing_key Defines the routing key of the binding. Only messages
   * with matching routing key will be delivered to the queue from the exchange.
   * Defaults to `""` which means all messages will be delivered.
   */
  void queueBind(const std::string& queue_name,
                 const std::string& exchange_name,
                 const std::string& routing_key = "");

  /**
   * Unbinds a queue from an exchange
   *
   * Disconnects a queue from an exchange.
   * @param queue_name The name of the queue to unbind.
   * @param exchange_name The name of the exchange to unbind.
   * @param routing_key This must match the routing_key of the binding.
   * @see BindQueue
   */
  void queueUnbind(const std::string& queue_name,
                   const std::string& exchange_name,
                   const std::string& routing_key = "");

  /**
   * Publishes a Basic message
   *
   * Publishes a Basic message to an exchange
   * @param exchange_name The name of the exchange to publish the message to
   * @param routing_key The routing key to publish with, this is used to route
   * to corresponding queue(s).
   * @param message The \ref BasicMessage object to publish to the queue.
   * @param mandatory Requires the message to be delivered to a queue.
   * @param immediate Requires the message to be both routed to a queue, and
   * immediately delivered to a consumer.
   */
  void basicPublish(const std::string& exchange,
                    const std::string& routing_key,
                    const AmqpMessage::Ptr message,
                    bool mandatory = false,
                    bool immediate = false);

  /**
   * Starts consuming Basic messages on a queue
   *
   * Subscribes as a consumer to a queue, so all future messages on a queue
   * will be Basic.Delivered
   * @note Due to a limitation to how things are done, it is only possible to
   * reliably have **a single consumer per channel**; calling this
   * more than once per channel may result in undefined results.
   * @param queue The name of the queue to subscribe to.
   * @param consumer_tag The name of the consumer. This is used to do
   * operations with a consumer.
   * @param no_local Defaults to true
   * @param no_ack If `true`, ack'ing the message is automatically done when the
   * message is delivered. Defaults to `true` (message does not have to be
   * ack'ed).
   * @param exclusive Means only this consumer can access the queue.
   * @param message_prefetch_count Number of unacked messages the broker will
   * deliver. Setting this to more than 1 will allow the broker to deliver
   * messages while a current message is being processed. A value of
   * 0 means no limit. This option is ignored if `no_ack = true`.
   * @returns the consumer tag
   */
  std::string basicConsume(const std::string& queue_name,
                           const std::string& consumer_tag = "",
                           bool no_local = false,
                           bool no_ack = true,
                           bool exclusive = false);

  /**
   * Cancels a previously created Consumer
   *
   * Unsubscribes a consumer from a queue. In other words undoes what
   * \ref basicConsume does.
   * @param consumer_tag The same `consumer_tag` used when the consumer was
   * created with \ref basicConsume.
   * @see basicConsume
   */
  void basicCancel(const std::string& consumer_tag);

  /**
   * Modify consumer's message prefetch count
   *
   * Sets the number of unacknowledged messages that will be delivered
   * by the broker to a consumer.
   *
   * Has no effect for consumer with `no_ack` set.
   *
   * @param consumer_tag The consumer tag to adjust the prefetch for.
   * @param message_prefetch_count The number of unacknowledged message the
   * broker will deliver. A value of 0 means no limit.
   */
  void basicQos(uint32_t prefetch_size, uint16_t prefetch_count, bool global);

  /**
   * Acknowledges a Basic message
   *
   * Acknowledges a message delievered using \ref BasicGet or \ref BasicConsume.
   * @param message The message that is being ack'ed.
   */
  void basicAck(uint64_t delivery_tag, bool multiple = false);

  /**
   * Reject (NAck) a Basic message
   */
  void basicNack(uint64_t delivery_tag,
                 bool multiple = false,
                 bool requeue = false);
  /**
   * Consumes a single message
   *
   * Waits for a single Basic message to be Delivered.
   *
   * This function only works after `BasicConsume` has successfully been called.
   *
   * @param consumer_tag Consumer ID (returned from \ref BasicConsume).
   * @returns The next message on the queue
   */
  AmqpEnvelope::Ptr basicConsumeMessage(const struct timeval* timeout);

  static Ptr createInstance(const AmqpConnection::Ptr conn)
  {
    return std::make_shared<AmqpChannel>(conn);
  }

private:
  AmqpChannel() = delete;

  struct Impl;
  /// PIMPL idiom
  std::unique_ptr<Impl> m_impl;
};

} // end namespace amqp

class MessageBroker
{
  ///< `"request"` string constant
  static const char* MESSAGE_TYPE_REQUEST;

  ///< `"response"` string constant
  static const char* MESSAGE_TYPE_RESPONSE;

  ///< `"error"` string constant
  static const char* MESSAGE_TYPE_ERROR;

public:
  using Ptr = std::shared_ptr<MessageBroker>;
  using WPtr = std::weak_ptr<MessageBroker>;
  using Properties = amqp::AmqpProperties;
  using Message = amqp::AmqpMessage;

  struct Configuration
  {
    struct
    {
      std::string name = "";
      std::string type = "";
      bool passive = false;
      bool durable = false;
      bool auto_delete = false;
      bool internal = false;
      bool declare = false;
    } exchange;
    struct
    {
      std::string name = "";
      bool passive = false;
      bool durable = false;
      bool auto_delete = false;
      bool exclusive = false;
      bool declare = false;
      bool bind = false;
    } queue;
    std::string routing_key = "";
    Properties properties;
  };

  /// An AMQP message class intended for a "Request/Reply" pattern. Use to build
  /// an RPC system: a client and a scalable RPC server.
  ///
  class Request : public Message
  {
  public:
    using Ptr = std::shared_ptr<Request>;
    using WPtr = std::weak_ptr<Request>;

    static Ptr createInstance() { return std::make_shared<Request>(); }
  };

  /// An AMQP message class intended for a "Request/Reply" pattern. Use to build
  /// an RPC system: a client and a scalable RPC server.
  ///
  class Response : public Message
  {
  public:
    using Ptr = std::shared_ptr<Response>;
    using WPtr = std::weak_ptr<Response>;

    inline bool ok() const { return properties().type != MESSAGE_TYPE_ERROR; }

    static Ptr createInstance() { return std::make_shared<Response>(); }
  };

  /**
   * @brief Parse a connection URL and establish an amqp connection.
   * An amqp connection url takes the form:
   * amqp://[$USERNAME[:$PASSWORD]\@]$HOST[:$PORT]/[$VHOST]
   * Examples:
   *    amqp://guest:guest\@localhost:5672//
   *	  amqp://guest:guest\@localhost/myvhost
   * Any missing parts of the URL will be set to the defaults specified in
   * amqp_default_connection_info.
   */
  MessageBroker(const std::string& url, int frame_max = 131072);

  /**
   * @brief Establish an amqp connection by parameters used to connect to the
   * RabbitMQ broker
   *
   * @param[in]  host       The port (i.e. 127.0.0.1)
   * @param[in]  port       The host (i.e. 5672)
   * @param[in]  username   The username (i.e. guest)
   * @param[in]  password   The password (i.e. guest)
   * @param[in]  vhost      The vhost virtual host to connect to on the broker.
   *                        The default on most brokers is "/"
   * @param[in]  frame_max  The maximum size of an AMQP frame on the wire to
   *                        request of the broker for this connection
   */
  MessageBroker(const std::string& host,
                int port,
                const std::string& username,
                const std::string& password,
                const std::string& vhost,
                int frame_max = 131072);

  virtual ~MessageBroker();

  /// Basic messaging pattern for publish events.
  ///
  /// @param[in]  configuration  The configuration
  /// @param[in]  messagebody    The messagebody
  ///
  void publish(const Configuration& configuration,
               const std::string& messagebody);

  /// RPC messaging pattern for publish events.
  ///
  /// @param[in]  configuration  The configuration
  /// @param[in]  messagebody    The messagebody
  /// @param      timeout        The timeout
  ///
  /// @return     { description_of_the_return_value }
  ///
  Response::Ptr publish(const Configuration& configuration,
                        const std::string& messagebody,
                        struct timeval* timeout);

  /// Basic messaging pattern for event subscription.
  ///
  /// @param[in]  configuration  The configuration
  /// @param[in]  callback       The callback
  ///
  void subscribe(const Configuration& configuration,
                 std::function<void(const Message::Ptr)> callback);

  /// RPC messaging pattern for event subscription.
  ///
  /// @param[in]  configuration  The configuration
  /// @param[in]  callback       The callback
  ///
  void subscribe(
    const Configuration& configuration,
    std::function<bool(const Request::Ptr, Response::Ptr)> callback);

  /// Close all subscription and join threads.
  ///
  void close();

  /// Generate random id
  static const std::string generateReqId();

protected:
  std::tuple<std::string, std::string> setupBroker(
    const Configuration& cfg,
    amqp::AmqpChannel::Ptr channel);

private:
  struct Impl;
  /// PIMPL idiom
  std::unique_ptr<Impl> m_impl;
};

} // end namespace gs

#endif // MESSAGE_BROKER_H
