// Some copyright should be here...

using System;
using System.IO;

using UnrealBuildTool;

public class DirectInputFFB : ModuleRules
{
	public DirectInputFFB(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            string WinPathDev = "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x64";
            PublicAdditionalLibraries.Add(Path.Combine(WinPathDev, "dinput8.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(WinPathDev, "dxguid.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(WinPathDev, "comctl32.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(WinPathDev, "winmm.lib"));
        }
        /*  new string[]
          {
              "dinput8.lib",
              "dxguid.lib",
              "comctl32.lib",
              "winmm.lib",}
          );*/

        PublicIncludePaths.AddRange(
			new string[] {
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
