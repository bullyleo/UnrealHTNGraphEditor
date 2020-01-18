// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HTNGraphEditor : ModuleRules
{
	public HTNGraphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				"Editor/GraphEditor/Public",
				"Editor/AIGraph/Public",
				"Editor/BehaviorTreeEditor/Public",
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"HTNGraphEditor/Private",
			}
			);

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetRegistry",
				"AssetTools",
				"PropertyEditor",
				"ContentBrowser",
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"HTNPlanner",
				// ... add other public dependencies that you statically link with here ...
			}
			);


		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"ApplicationCore",
				"Engine",
				"RenderCore",
				"InputCore",
				"Slate",
				"SlateCore",
				"EditorStyle",
				//"UnrealEd",
				"AudioEditor",
				"MessageLog",
				"GraphEditor",
				"Kismet",
				"KismetWidgets",
				"PropertyEditor",
				"AnimGraph",
				"BlueprintGraph",
				"AIGraph",
				"AIModule",
				"ClassViewer",
				"ToolMenus",
				"HTNGraphRuntime",
				// ... add private dependencies that you statically link with here ...	
			}
			);


		PublicIncludePathModuleNames.Add("LevelEditor");

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"WorkspaceMenuStructure",
				"AssetTools",
				"AssetRegistry",
				"ContentBrowser"
			}
		);
	}
}
