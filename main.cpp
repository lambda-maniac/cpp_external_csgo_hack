#include "MemoryManager.hpp"
#include "_3D2D.hpp"

// Initialization:
#define WINDOW_NAME "Counter-Strike: Global Offensive - Direct3D 9"

MemoryManager mm = MemoryManager(WINDOW_NAME);

DWORD client = mm.getModuleBaseAddress("client.dll");
DWORD engine = mm.getModuleBaseAddress("engine.dll");

Vector2 windowSize = Vector2(1600, 900);

// General functions:
DWORD getPlayer(int index)
{ return mm.read<DWORD>(client + dwEntityList + index * 0x10);
}

int getPlayerHealth(DWORD player)
{ return mm.read<int>(player + m_iHealth);
}

int getPlayerTeam(DWORD player)
{ return mm.read<int>(player + m_iTeamNum);
}

int whoSpottedByMask(DWORD player) 
{ return mm.read<int>(player + m_bSpottedByMask);
}

DWORD getPlayerBoneMatrix(DWORD player)
{ return mm.read<DWORD>(player + m_dwBoneMatrix);
}

Vector3 getPlayerBonePosition(DWORD player, int bone)
{
    DWORD boneMatrix = getPlayerBoneMatrix(player);

    return Vector3 (
        mm.read<float>(boneMatrix + 0x30 * bone + 0x0C),
        mm.read<float>(boneMatrix + 0x30 * bone + 0x1C),
        mm.read<float>(boneMatrix + 0x30 * bone + 0x2C)
    );
}

Vector3 getPlayerPosition(DWORD player)
{
    return Vector3 (
        mm.read<float>(player + m_vecOrigin + 0x0),
        mm.read<float>(player + m_vecOrigin + 0x4),
        mm.read<float>(player + m_vecOrigin + 0x8)
    );
}

// Local player functions:
DWORD getLocalPlayer(void)
{ return mm.read<DWORD>(client + dwLocalPlayer);
}

DWORD getClientState(void)
{ return mm.read<DWORD>(engine + dwClientState);
}

int getLocalPlayerFlags(void)
{ return mm.read<int>(getLocalPlayer() + m_fFlags);
}

int getLocalPlayerId(void)
{ return mm.read<int>(getClientState() + dwClientState_GetLocalPlayer);
}

Angle getLocalPlayerAimPunchAngles(void)
{
    DWORD localPlayer = getLocalPlayer(); 
    
    return Angle (
        mm.read<float>(localPlayer + m_aimPunchAngle + 0x0),
        mm.read<float>(localPlayer + m_aimPunchAngle + 0x4)
    );
}

Angle getLocalPlayerViewAngles(void)
{
    DWORD clientState = getClientState();

    return Angle (
        mm.read<float>(clientState + dwClientState_ViewAngles + 0x0),
        mm.read<float>(clientState + dwClientState_ViewAngles + 0x4)
    );
}

Vector3 getLocalPlayerViewOffset(void)
{
    DWORD localPlayer = getLocalPlayer();

    return Vector3 (
        mm.read<float>(localPlayer + m_vecViewOffset + 0x0),
        mm.read<float>(localPlayer + m_vecViewOffset + 0x4),
        mm.read<float>(localPlayer + m_vecViewOffset + 0x8)
    );
}

Vector3 getLocalPlayerPosition(void)
{ return getPlayerPosition(getLocalPlayer());
}

void getLocalPlayerViewMatrix(float (&viewMatrix)[16])
{
    for (int n = 0; n < 16; ++n)
        viewMatrix[n] = mm.read<float>(client + dwViewMatrix + (0x04 * n));
}

void writeLocalPlayerViewAngles(Angle angle)
{
    angle.normalize();

    DWORD clientState = getClientState();

    mm.write<float>(clientState + dwClientState_ViewAngles + 0x0, angle.pitch);
    mm.write<float>(clientState + dwClientState_ViewAngles + 0x4, angle.yaw  );
}

void forceLocalPlayerAimTo(Vector3 target)
{
    Vector3 localPlayerHeadPosition = getLocalPlayerPosition()
                                    + getLocalPlayerViewOffset();

    Vector3 delta = target 
                  - localPlayerHeadPosition;

    float deltaLength = localPlayerHeadPosition.distanceTo(target);

    Angle punchAngles = getLocalPlayerAimPunchAngles() * 2.1;

    Angle angle = Angle (
        (-asin (delta.z / deltaLength) * (180.0f / PI) - punchAngles.pitch),
        ( atan2(delta.y , delta.x    ) * (180.0f / PI) - punchAngles.yaw  )
    );

    writeLocalPlayerViewAngles(angle);
}

// Predicates:
bool isDormant(DWORD player)
{ return mm.read<int>(player + m_bDormant);
}

bool isDead(DWORD player)
{ int health = getPlayerHealth(player)
; return (health < 1 || health > 100);
}

bool isTeamEqual(DWORD playerA, DWORD playerB)
{ return getPlayerTeam(playerA) == getPlayerTeam(playerB);
}

bool isVisible(DWORD player)
{ return whoSpottedByMask(player) & (1 << getLocalPlayerId());
}

// Main functions:
int findClosestValidEnemy(void)
{
    float closestDistance      = 99999.9f ;
    short closestDistanceIndex = -1       ;

    DWORD localPlayer = getLocalPlayer();

    for (short index = 1; index < 32; ++index)
    {
        DWORD entity = getPlayer(index);

        if 
        (   entity == NULL
        ||  isTeamEqual(localPlayer, entity)
        || !isVisible(entity)
        ||  isDormant(entity)
        ||  isDead   (entity)
        )   continue;

        float currentDistance = 
            getPlayerPosition(localPlayer).distanceTo(getPlayerPosition(entity));

        if (currentDistance < closestDistance)
        {
            closestDistance      = currentDistance ;
            closestDistanceIndex = index           ;
        }
    }

    return closestDistanceIndex;
}

void aimbot(void)
{
    int entityIndex = findClosestValidEnemy();

    if (entityIndex != -1)
    {
        forceLocalPlayerAimTo(
            getPlayerBonePosition(
                getPlayer(entityIndex), 8
            )
        );
    }
}

void triggerbot(void)
{
    DWORD localPlayer = getLocalPlayer();

    int entityId = mm.read<int>(localPlayer + m_iCrosshairId);
    DWORD entity = getPlayer(entityId - 1);

    if 
    (  entityId > 0 
    && entityId <= 64 
    && !isTeamEqual(localPlayer, entity)
    )
        mm.write<int>(client + dwForceAttack, 6);
}

void drawLine(HDC hdc, POINT A, POINT B, COLORREF color)
{
    HPEN pen     = CreatePen(PS_SOLID, 2, color);
    HPEN old_pen = (HPEN) SelectObject(hdc, pen);

    MoveToEx(hdc, A.x, A.y, NULL);
    LineTo  (hdc, B.x, B.y);

    SelectObject(hdc, old_pen);
    DeleteObject(pen);
}

bool worldToScreenCoordinates(Vector3 position, float matrix[16], Vector2 windowSize, Vector2& onScreen)
{
    Vector4 clipCoordinates = Vector4 (
        position.x * matrix[ 0] + position.y * matrix[ 1] + position.z * matrix[ 2] + matrix[ 3],
        position.x * matrix[ 4] + position.y * matrix[ 5] + position.z * matrix[ 6] + matrix[ 7],
        position.x * matrix[ 8] + position.y * matrix[ 9] + position.z * matrix[10] + matrix[11],
        position.x * matrix[12] + position.y * matrix[13] + position.z * matrix[14] + matrix[15]
    );

    if (clipCoordinates.w < 0.1f) return false;

    Vector2 normalizedDeviceCoordinates = Vector2 (
        clipCoordinates.x / clipCoordinates.w,
        clipCoordinates.y / clipCoordinates.w
    );

    onScreen.x =  (windowSize.x / 2 * normalizedDeviceCoordinates.x) + (normalizedDeviceCoordinates.x + windowSize.x / 2);
    onScreen.y = -(windowSize.y / 2 * normalizedDeviceCoordinates.y) + (normalizedDeviceCoordinates.y + windowSize.y / 2);

    return true;
}

void wallhack(HDC hdc)
{
    DWORD localPlayer = getLocalPlayer();

    for (short index = 1; index < 32; ++index)
    {
        DWORD entity = getPlayer(index);

        if
        (   entity == NULL
        ||  entity == localPlayer
        ||  isDormant(entity)
        ||  isDead   (entity)
        )   continue;

        float viewMatrix[16];
        getLocalPlayerViewMatrix(viewMatrix);

        Vector2 onScreen = Vector2(0, 0);
        if (worldToScreenCoordinates(getPlayerPosition(entity), viewMatrix, windowSize, onScreen))
        {
            drawLine (
                hdc, 
                {(long) windowSize.x / 2, (long) windowSize.y}, 
                {(long) onScreen.x      , (long) onScreen.y  },
                (isTeamEqual(localPlayer, entity) ? RGB(0, 0, 255) : RGB(255, 0, 0))
            );
        }
    }
}

bool isKeyDown(int vKey)
{ return (GetAsyncKeyState(vKey) & 0x8000) != 0;
}

int main(void)
{
    const HDC hdc = GetDC(mm.hWnd);

    windowSize.__cout();

    while (!isKeyDown(VK_END))
    {
        if (isKeyDown(VK_SHIFT))
        {
            aimbot();
            triggerbot();
        }

        wallhack(hdc);
    }

    return 0;
}