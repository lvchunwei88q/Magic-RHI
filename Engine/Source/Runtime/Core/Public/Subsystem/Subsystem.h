#pragma once
#include "Common/CORE_API.h"
#include "Common/Check.h"

#include "Subsystem/SubsystemContext.h"

#include <typeinfo> 

FORBIDDEN_METHOD_CONCEPT(Subsystem, Init);
FORBIDDEN_METHOD_CONCEPT(Subsystem, Uninstall);
FORBIDDEN_METHOD_CONCEPT(Subsystem, Notification);

template<typename T>
concept COMBINE_FORBIDDEN_NAME(Subsystem) = COMBINE_FORBIDDEN_METHODS(Subsystem, Init) ||
	COMBINE_FORBIDDEN_METHODS(Subsystem, Uninstall) || COMBINE_FORBIDDEN_METHODS(Subsystem, Notification);

namespace Core
{
	CORE_API SubsystemContext* GetSubsystemContext();

	// Subsystem base class
	class CORE_API Subsystem
	{
	public:
		Subsystem() = default;

		// api
		virtual bool Init() = 0;
		virtual void Uninstall() = 0;
		virtual void Notification(NotificationContext) {}; // Notification

		template<typename Derived, Priority Priority>
		void Register(const char* tags = "NONE") {
			// Register the subsystem in the system manager
			const char* name = typeid(Derived).name();

			SubsystemContext::Context context;
			context.name = name;
			context.tags = tags;
			context.subsystem = this;
			context.priority = Priority; // Default priority, can be customized

			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->RegisterSubsystem(context);
		}
	};

	class CORE_API SubsystemControl
	{
	public:
		static SubsystemError Init() {
			// Initialize all registered subsystems
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			// Subsystemcontext->Init(); // This would be an API to initialize all subsystems
			return Subsystemcontext->Init();
		}

		static void Uninstall() {
			// Uninstall all registered subsystems
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->Uninstall();
		}

		static void Register_Init_Callback(Init_Callback func) {
			// Register Init _Callback Function
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->RegisterInitCallbackFunction(func);
		}

		static size_t GetNum() {
			// Get Subsystem Num
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			return Subsystemcontext->Num();
		}

		static void NotificationSubsystem(const char* target_tags, NotificationContext Context) {
			// Notification Subsystem Msg
			SubsystemContext* Subsystemcontext = GetSubsystemContext();
			Subsystemcontext->Notification(target_tags, Context);
		}
	};
}
// We use a nickname to make it more recognizable
using SubSystem = Core::Subsystem;