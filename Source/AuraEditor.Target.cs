// 版权归暮志未晚所有。

using UnrealBuildTool;
using System.Collections.Generic;

public class AuraEditorTarget : TargetRules
{
	public AuraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "Aura" } );
	}
}
