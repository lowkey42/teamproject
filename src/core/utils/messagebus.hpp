#pragma once

#include "func_traits.hpp"
#include "reflection.hpp"
#include "log.hpp"
#include "maybe.hpp"
#include "template_utils.hpp"

#include <moodycamel/concurrentqueue.hpp>

#include <vector>
#include <unordered_map>
#include <cassert>
#include <functional>
#include <algorithm>
#include <utility>

namespace mo {
namespace util {

	class Message_bus;

	constexpr auto default_mailbox_size = 128;
	constexpr auto default_msg_batch_size = 4;

	template<class T>
	class Mailbox {
		public:
			Mailbox(Message_bus& bus, std::size_t size=default_msg_batch_size);
			Mailbox(Message_bus&&) = delete;
			~Mailbox();

			void send(const T& v);
			auto receive() -> maybe<T>;
			template<std::size_t Size>
			auto receive(T (&target)[Size]) -> std::size_t;

			auto empty()const noexcept -> bool;

		private:
			moodycamel::ConcurrentQueue<T> _queue;
			Message_bus& _bus;
	};

	class Mailbox_collection {
		public:
			Mailbox_collection(Message_bus& bus);

			template<class T, std::size_t bulk_size, typename Func>
			void subscribe(std::size_t queue_size, Func handler={});

			template<std::size_t bulk_size=default_msg_batch_size,
			         std::size_t queue_size=default_mailbox_size,
			         typename... Func>
			void subscribe_to(Func&&... handler);

			template<class T>
			void unsubscribe();

			template<class T, std::size_t size, typename Func>
			void receive(Func handler);

			void update_subscriptions();

			template<typename Msg, typename... Arg>
			void send(Arg&&... arg) {
				send<Msg>(util::typeuid_of<void>(), std::forward<Arg>(arg)...);
			}
			template<typename Msg, typename... Arg>
			void send(Typeuid self, Arg&&... arg) {
				send_msg(Msg{std::forward<Arg>(arg)...}, self);
			}

			template<typename Msg>
			void send_msg(const Msg& msg, Typeuid self);

		private:
			struct Sub {
				std::shared_ptr<void> box;
				std::function<void(Sub&)> handler;
			};

			Message_bus& _bus;
			std::unordered_map<Typeuid, Sub> _boxes;
	};


	class Message_bus {
		public:
			Message_bus();
			Message_bus(Message_bus&&) = default;
			~Message_bus();
			auto create_child() -> Message_bus;

			template<typename T>
			void register_mailbox(Mailbox<T>& mailbox, Typeuid self=0);
			template<typename T>
			void unregister_mailbox(Mailbox<T>& mailbox);

			void update();

			template<typename Msg, typename... Arg>
			void send(Arg&&... arg) {
				send_others<Msg>(util::typeuid_of<void>(), std::forward<Arg>(arg)...);
			}
			template<typename Msg, typename... Arg>
			void send_others(Typeuid self, Arg&&... arg) {
				send_msg(Msg{std::forward<Arg>(arg)...}, self);
			}

			template<typename Msg>
			void send_msg(const Msg& msg, Typeuid self);

		private:
			Message_bus(Message_bus* parent);

			struct Mailbox_ref {
				template<typename T>
				Mailbox_ref(Mailbox<T>& mailbox, Typeuid self=0) ;

				template<typename T>
				void exec_send(const T& m, Typeuid self);

				bool operator==(const Mailbox_ref& rhs)const noexcept {
					return _type==rhs._type && _mailbox==rhs._mailbox;
				}

				Typeuid _self;
				Typeuid _type;
				void* _mailbox;
				std::function<void(void*, const void*)> _send;
			};

			auto& group(Typeuid id) {
				if(std::size_t(id)>=_mb_groups.size()) {
					_mb_groups.resize(id+1);
					_mb_groups.back().reserve(4);
				}
				return _mb_groups[id];
			}

			Message_bus* _parent;
			std::vector<Message_bus*> _children;

			std::vector<std::vector<Mailbox_ref>> _mb_groups;

			std::vector<Mailbox_ref> _add_queue;
			std::vector<Mailbox_ref> _remove_queue;
	};

}
}

#define MESSAGEBUS_HPP_INCLUDED
#include "messagebus.hxx"

