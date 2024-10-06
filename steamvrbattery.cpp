// ================== copperpixel ~ 2024 ==================
// 
// Purpose: SteamVR gamepad battery display
// 
// =========================================================

#include <cstdio> 
#include <cstring>
#include <csignal>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif //_WIN32

#include "valve/openvr.h"

vr::TrackedDeviceIndex_t g_iLeftGamepad = -1, g_iRightGamepad = -1;
vr::IVRSystem* g_pVRSystem = nullptr;

bool g_bRunning = true; 

uint32_t g_iUpdateInterval = 1000;
bool g_bVerbose = false;

void SignalHandler( int iSignal )
{
    if( iSignal == SIGINT )
        g_bRunning = false;
}

#ifdef _WIN32
BOOL WINAPI ConsoleHandler( DWORD dwCtrlType )
{
    if( dwCtrlType == CTRL_CLOSE_EVENT )
    {
        g_bRunning = false;
        return TRUE;
    }
    return FALSE;
}
#endif //_WIN32

int main( int argc, char** argv )
{
    // signals
    signal( SIGINT, SignalHandler );
#ifdef _WIN32
    SetConsoleCtrlHandler( ConsoleHandler, TRUE );
#endif //_WIN32

    // command line args
    for( int i = 0; i < argc; i++ )
    {
        if( !strcmp( argv[ i ], "--help" ) ||
            !strcmp( argv[ i ], "-?" ) )
        {
            printf( "steamvrbattery ~ copperpixel on GitHub\n"
                    "--help, -? : Shows this prompt\n"
                    "--interval <miliseconds>, -i <miliseconds> : Battery charge display update interval (default 1000ms)\n"
                    "--verbose, -v : Enable verbose output\n" );

            return 1;
        }
        else if( !strcmp( argv[ i ], "--interval" ) ||
                 !strcmp( argv[ i ], "-i" ) )
        {
            // make sure a number is passed to -i
            if( i + 1 >= argc )
            {
                printf( "No number passed to update interval argument\n"
                        "Use --help or -? argument to view help\n" );
                return -1;
            }

            char* pEnd;
            g_iUpdateInterval = ( uint32_t )strtoul( argv[ i + 1 ], &pEnd, 10 );
            if( *pEnd != '\0' )
            {
                printf( "Passed update interval is not a valid number\n"
                        "Use --help or -? argument to view help\n" );
                return -2;
            }

            i++; // skip next arg since we use it to pass the new update interval
        }
        else if( !strcmp( argv[ i ], "--verbose" ) ||
                 !strcmp( argv[ i ], "-v" ) )
            g_bVerbose = true;
    }

    // openvr initialization
    vr::HmdError pHmdError;

    if( g_bVerbose )
        printf( "Initializing OpenVR...\n\n" );

    g_pVRSystem = vr::VR_Init( &pHmdError, vr::VRApplication_Utility );
    if( pHmdError != vr::VRInitError_None )
    {
        printf( "Failed to initialize OpenVR : \n%s\n", vr::VR_GetVRInitErrorAsEnglishDescription( pHmdError ) );
        return -3;
    }

    // get indexes for both gamepads
    for( vr::TrackedDeviceIndex_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i )
    {
        if( g_iLeftGamepad != -1 && g_iRightGamepad != -1 )
            break;

        if( !g_pVRSystem->IsTrackedDeviceConnected( i ) )
            continue;
            
        vr::ETrackedDeviceClass eDeviceClass = g_pVRSystem->GetTrackedDeviceClass( i );

        if( eDeviceClass != vr::TrackedDeviceClass_Controller )
            continue;

        int32_t iControllerRole = g_pVRSystem->GetInt32TrackedDeviceProperty( i, vr::Prop_ControllerRoleHint_Int32 );

        switch( iControllerRole )
        {
        case vr::TrackedControllerRole_LeftHand:
            g_iLeftGamepad = i;

            if( g_bVerbose )
                printf( "Left gamepad index: %i\n", g_iLeftGamepad );

            break;
        case vr::TrackedControllerRole_RightHand:
            g_iRightGamepad = i;

            if( g_bVerbose )
                printf( "Right gamepad index: %i\n", g_iRightGamepad );

            break;
        }
    }

    if( g_iLeftGamepad == -1 || g_iRightGamepad == -1 )
    {
        printf( "Gamepads not found!\nThis error may also occur if you don't have SteamVR running\n" );
        vr::VR_Shutdown();
        return -4;
    }

    // battery charge display
    printf( "Current battery charge:\n" );
    while( g_bRunning )
    {
        float flLeftBattery = g_pVRSystem->GetFloatTrackedDeviceProperty( g_iLeftGamepad, vr::Prop_DeviceBatteryPercentage_Float );
        float flRightBattery = g_pVRSystem->GetFloatTrackedDeviceProperty( g_iRightGamepad, vr::Prop_DeviceBatteryPercentage_Float );

        if( g_bVerbose )
            printf( "\rLeft gamepad: %f    Right gamepad: %f    ", flLeftBattery, flRightBattery );
        else
            printf( "\rLeft gamepad: %.2f%%    Right gamepad: %.2f%%    ", flLeftBattery * 100, flRightBattery * 100 );

        fflush( stdout );

#ifdef _WIN32
        Sleep( g_iUpdateInterval );
#else
        usleep( g_iUpdateInterval * 1000 ); // usleep takes microseconds
#endif //_WIN32
    }

    if( g_bVerbose )
        printf( "\nShutting down OpenVR...\n" );

    vr::VR_Shutdown();

    return 0;
}
