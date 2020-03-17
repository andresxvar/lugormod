#include "g_local.h"
#include "Lmd_Console.h"

void Confirm_Clear(gentity_t *ent)
{
	G_Free(ent->client->Lmd.confirm.data);
	memset(&ent->client->Lmd.confirm, 0, sizeof(ent->client->Lmd.confirm));
}

void Confirm_Check(gentity_t *ent)
{
	if (ent->client->Lmd.confirm.time && ent->client->Lmd.confirm.time + 30000 <= level.time)
	{
		Disp(ent, CT_B "Your confirmation has timed out.");
		Confirm_Clear(ent);
	}
}

qboolean Confirm_Set(gentity_t *ent, void (*func)(gentity_t *ent, void *data), void *data)
{
	Confirm_Clear(ent);
	if (ent->client->pers.Lmd.persistantFlags & SPF_NOCONFIRM)
	{
		// Execute the command immediately.
		func(ent, data);
		return qfalse;
	}

	ent->client->Lmd.confirm.func = func;
	ent->client->Lmd.confirm.data = data;
	Disp(ent, CT_B "Confirmation command added\n" CT_B "Use \'" CT_C "/confirm yes" CT_B "\' to confirm the command or \'" CT_C "/confirm no" CT_B "\' to cancel.\n" CT_B "Use \'" CT_C "/confirm toggle" CT_B "\' to disable the confirmation requirement.");
	return qtrue;
}

void Cmd_Confirm_f(gentity_t *ent, int iArg)
{
	char arg[MAX_STRING_CHARS];
	trap_Argv(1, arg, sizeof(arg));
	if (Q_stricmp(arg, "yes") == 0)
	{
		if (!ent->client->Lmd.confirm.func)
		{
			Disp(ent, CT_B "You have no pending confirmation.");
			return;
		}
		ent->client->Lmd.confirm.func(ent, ent->client->Lmd.confirm.data);
		Disp(ent, CT_S "Confirmed.");
		Confirm_Clear(ent);
	}
	else if (Q_stricmp(arg, "no") == 0)
	{
		if (!ent->client->Lmd.confirm.func)
		{
			Disp(ent, CT_B "You have no pending confirmation.");
			return;
		}
		Disp(ent, CT_B "Confirmation canceled.");
		Confirm_Clear(ent);
	}
	else if (Q_stricmp(arg, "toggle") == 0)
	{
		if (ent->client->pers.Lmd.persistantFlags & SPF_NOCONFIRM)
		{
			Disp(ent, CT_B "Confirmation is enabled. You will now be asked to confirm certain commands.");
			ent->client->pers.Lmd.persistantFlags &= ~SPF_NOCONFIRM;
		}
		else
		{
			Disp(ent, CT_B "Confirmation is disabled.  You will not be asked to confirm commands.");
			if (ent->client->Lmd.confirm.func)
			{
				Disp(ent, CT_B "You still need to confirm or cancel your pending command.");
			}
			ent->client->pers.Lmd.persistantFlags |= SPF_NOCONFIRM;
		}
	}
	else
		Disp(ent, CT_B "Usage: " CT_C "/confirm " CT_AR "<\'yes\' | \'no\' | \'toggle\'>");
}