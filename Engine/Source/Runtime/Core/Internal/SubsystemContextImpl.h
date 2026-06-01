#pragma once
#include "Subsystem/SubsystemContext.h"
#include "Subsystem/Subsystem.h"
#include "Tools/Singleton.h"

#include <vector>
#include <string>

namespace Core
{
	class SubsystemContextImpl : public SubsystemContext, public Singleton<SubsystemContextImpl>
	{
		struct Context
		{
			std::string name;
			std::string tags;
			Subsystem* subsystem;
			Priority priority; 
		};
	public:
		SubsystemContextImpl();

		virtual void RegisterSubsystem(SubsystemContext::Context context) override;
		virtual SubsystemError Init() override;
		virtual void Uninstall() override;
		virtual void Notification(const char* target_tags, NotificationContext msg) override; // Notification Subsystem

		// Register Init Callback Function
		virtual void RegisterInitCallbackFunction(Init_Callback Func) override;
		// Get Subsystem Num
		virtual size_t Num() override;
	private:
		std::vector<Context> Derived;
		std::vector<Init_Callback> IC_Functions;
	};
}