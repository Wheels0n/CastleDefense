#pragma once
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
#pragma comment(lib, "Debug\\libprotobufd.lib")
#pragma comment(lib, "utf8_validity.lib")
#pragma comment(lib, "Recast-d.lib")
#pragma comment(lib, "DebugUtils-d.lib")
#pragma comment(lib, "Detour-d.lib")
#pragma comment(lib, "DetourCrowd-d.lib")
#pragma comment(lib, "DetourTileCache-d.lib")
#else
#pragma comment(lib, "Release\\ServerCore.lib")
#pragma comment(lib, "Release\\libprotobuf.lib")
#pragma comment(lib, "utf8_validity.lib")
#pragma comment(lib, "Recast.lib")
#pragma comment(lib, "DebugUtils.lib")
#pragma comment(lib, "Detour.lib")
#pragma comment(lib, "DetourCrowd.lib")
#pragma comment(lib, "DetourTileCache.lib")
#endif // _DEBUG

//#pragma comment(lib, "absl_bad_any_cast_impl.lib")
#pragma comment(lib, "absl_bad_optional_access.lib")
//#pragma comment(lib, "absl_bad_variant_access.lib")
#pragma comment(lib, "absl_base.lib")
#pragma comment(lib, "absl_city.lib")
//#pragma comment(lib, "absl_civil_time.lib")
#pragma comment(lib, "absl_cord.lib")
#pragma comment(lib, "absl_cord_internal.lib")
//#pragma comment(lib, "absl_cordz_functions.lib")
#pragma comment(lib, "absl_cordz_handle.lib")
#pragma comment(lib, "absl_cordz_info.lib")
//#pragma comment(lib, "absl_cordz_sample_token.lib")
#pragma comment(lib, "absl_crc_cord_state.lib")
//#pragma comment(lib, "absl_crc_cpu_detect.lib")
#pragma comment(lib, "absl_crc_internal.lib")
#pragma comment(lib, "absl_crc32c.lib")
//#pragma comment(lib, "absl_debugging_internal.lib")
//#pragma comment(lib, "absl_demangle_internal.lib")
//#pragma comment(lib, "absl_die_if_null.lib")
#pragma comment(lib, "absl_examine_stack.lib")
//#pragma comment(lib, "absl_exponential_biased.lib")
//#pragma comment(lib, "absl_failure_signal_handler.lib")
//#pragma comment(lib, "absl_flags_commandlineflag.lib")
//#pragma comment(lib, "absl_flags_commandlineflag_internal.lib")
//#pragma comment(lib, "absl_flags_config.lib")
//#pragma comment(lib, "absl_flags_internal.lib")
//#pragma comment(lib, "absl_flags_marshalling.lib")
//#pragma comment(lib, "absl_flags_parse.lib")
//#pragma comment(lib, "absl_flags_private_handle_accessor.lib")
//#pragma comment(lib, "absl_flags_program_name.lib")
//#pragma comment(lib, "absl_flags_reflection.lib")
//#pragma comment(lib, "absl_flags_usage.lib")
//#pragma comment(lib, "absl_flags_usage_internal.lib")
#pragma comment(lib, "absl_graphcycles_internal.lib")
#pragma comment(lib, "absl_hash.lib")
//#pragma comment(lib, "absl_hashtablez_sampler.lib")
#pragma comment(lib, "absl_int128.lib")
#pragma comment(lib, "absl_kernel_timeout_internal.lib")
//#pragma comment(lib, "absl_leak_check.lib")
//#pragma comment(lib, "absl_log_entry.lib")
//#pragma comment(lib, "absl_log_flags.lib")
#pragma comment(lib, "absl_log_globals.lib")
//#pragma comment(lib, "absl_log_initialize.lib")
#pragma comment(lib, "absl_log_internal_check_op.lib")
#pragma comment(lib, "absl_log_internal_conditions.lib")
//#pragma comment(lib, "absl_log_internal_fnmatch.lib")
#pragma comment(lib, "absl_log_internal_format.lib")
#pragma comment(lib, "absl_log_internal_globals.lib")
#pragma comment(lib, "absl_log_internal_log_sink_set.lib")
#pragma comment(lib, "absl_log_internal_message.lib")
#pragma comment(lib, "absl_log_internal_nullguard.lib")
#pragma comment(lib, "absl_log_internal_proto.lib")
//#pragma comment(lib, "absl_log_severity.lib")
#pragma comment(lib, "absl_log_sink.lib")
#pragma comment(lib, "absl_low_level_hash.lib")
#pragma comment(lib, "absl_malloc_internal.lib")
//#pragma comment(lib, "absl_periodic_sampler.lib")
//#pragma comment(lib, "absl_random_distributions.lib")
//#pragma comment(lib, "absl_random_internal_platform.lib")
//#pragma comment(lib, "absl_random_internal_pool_urbg.lib")
//#pragma comment(lib, "absl_random_internal_randen.lib")
//#pragma comment(lib, "absl_random_internal_randen_hwaes.lib")
//#pragma comment(lib, "absl_random_internal_randen_hwaes_impl.lib")
//#pragma comment(lib, "absl_random_internal_randen_slow.lib")
//#pragma comment(lib, "absl_random_internal_seed_material.lib")
//#pragma comment(lib, "absl_random_seed_gen_exception.lib")
//#pragma comment(lib, "absl_random_seed_sequences.lib")
#pragma comment(lib, "absl_raw_hash_set.lib")
#pragma comment(lib, "absl_raw_logging_internal.lib")
//#pragma comment(lib, "absl_scoped_set_env.lib")
#pragma comment(lib, "absl_spinlock_wait.lib")
#pragma comment(lib, "absl_stacktrace.lib")
#pragma comment(lib, "absl_status.lib")
#pragma comment(lib, "absl_statusor.lib")
#pragma comment(lib, "absl_str_format_internal.lib")
#pragma comment(lib, "absl_strerror.lib")
#pragma comment(lib, "absl_string_view.lib")
#pragma comment(lib, "absl_strings.lib")
#pragma comment(lib, "absl_strings_internal.lib")
#pragma comment(lib, "absl_symbolize.lib")
#pragma comment(lib, "absl_synchronization.lib")
#pragma comment(lib, "absl_throw_delegate.lib")
#pragma comment(lib, "absl_time.lib")
#pragma comment(lib, "absl_time_zone.lib")
//#pragma comment(lib, "absl_vlog_config_internal.lib")

//#pragma comment(lib, "utf8_range.lib")
#define _CRT_SECURE_NO_WARNINGS

#include "Memory.h"
#include "Session.h"
#include "SessionManager.h"
#include "PlayerManager.h"
#include "EnemyManager.h"
#include "NavigationManager.h"
#include "IocpManager.h"

int main()
{
	
	g_pMemoryPoolManager = new MemoryPoolManager();
	g_pSessionManager = new SessionManager();
	g_pPlayerManager = new PlayerManager();
	g_pNavManager = new NavigationManager();
	g_pEnemyManager = new EnemyManager();
	g_pIocpManager = new IocpManager();

	if (!g_pIocpManager->Init())
	{
		return -1;
	}
	
	if (!g_pIocpManager->StartListen())
	{
		return -1;
	}

	g_pIocpManager->RunIOThreads();
	g_pIocpManager->StartAccept();

	delete g_pIocpManager;
	delete g_pSessionManager;
	delete g_pPlayerManager;
	delete g_pEnemyManager;
	delete g_pMemoryPoolManager;

	return 0;
}