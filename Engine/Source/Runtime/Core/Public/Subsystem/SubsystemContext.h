#pragma once
namespace Core
{
	using Init_Callback = void(*)(const char*, size_t, size_t); // Get Subsystem name , name size , index

	enum class Priority
	{
		Low = -1,      // 低优先级
		Normal = 0,    // 普通优先级
		High = 1,      // 高优先级
		Core = 2,      // 核心优先级
	};

	struct NotificationContext
	{
		size_t tags; // Target tags for notification
		const char* msg; // Notification message

		NotificationContext(size_t tags, const char* msg) : tags(tags), msg(msg) {}
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