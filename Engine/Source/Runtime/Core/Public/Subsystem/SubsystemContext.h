#pragma once
namespace Core
{
	using Init_Callback = void(*)(const char*, size_t, size_t); // Get Subsystem name , name size , index

	// This is an enum used to set priorities, and we use it to set the initialization priority of the Subsystem.
	enum class Priority
	{
		Low = -1,      // Low priority
		Normal = 0,    // Normal priority
		High = 1,      // High priority
		Core = 2,      // Core priority
	};

	struct NotificationContext
	{
		size_t tags; // Target tags for notification
		const char* msg; // Notification message

		// No implicit conversion
		explicit NotificationContext(size_t tags, const char* msg) : tags(tags), msg(msg) {}
		// copy
		NotificationContext(const NotificationContext& other)
			: tags(other.tags), msg(other.msg) {}
	};

	struct SubsystemError {
		bool error;
		const char* target;
	};

	class SubsystemContext
	{
	public:
		struct Context
		{
			const char* name;
			const char* tags; //subsystem notification tags
			void* subsystem; // Pointer to the subsystem instance
			Priority priority; // priority
		};

		// API
		virtual void RegisterSubsystem(Context context) = 0;
		virtual SubsystemError Init() = 0;
		virtual void Uninstall() = 0;
		virtual void Notification(const char* target, NotificationContext msg) = 0; // Notification Subsystem

		// Register Init Callback Function
		virtual void RegisterInitCallbackFunction(Init_Callback Func) = 0;
		// Get Subsystem Num
		virtual size_t Num() = 0;
	};
}

using NotifContext = Core::NotificationContext;
using SubSystemError = Core::SubsystemError;