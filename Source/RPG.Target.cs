// 版权归暮志未晚所有。

using UnrealBuildTool;
using System.Collections.Generic;

public class RPGTarget : TargetRules
{
	public RPGTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "RPG" } );
	}
}
