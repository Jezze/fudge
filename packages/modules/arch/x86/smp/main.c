#include <fudge.h>
#include <modules/arch/x86/acpi/acpi.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include "smp.h"

static struct smp_architecture architecture;

static void setmadt()
{

    unsigned long madttable;
    unsigned long madtend;
    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (!madt)
        return;

    madttable = (unsigned long)madt + sizeof (struct acpi_madt);
    madtend = (unsigned long)madt + madt->base.length;

    while (madttable < madtend)
    {

        struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

        if (entry->type == 0)
        {

            struct acpi_madt_lapic *lapic = (struct acpi_madt_lapic *)entry;

            architecture.cpus[architecture.count].id = lapic->id;
            architecture.count++;

        }

        madttable += entry->length;

    }

}

static void setsrat()
{

    unsigned long srattable;
    unsigned long sratend;
    struct acpi_srat *srat = (struct acpi_srat *)acpi_findheader("SRAT");

    if (!srat)
        return;

    srattable = (unsigned long)srat + sizeof (struct acpi_srat);
    sratend = (unsigned long)srat + srat->base.length;

    while (srattable < sratend)
    {

        struct acpi_srat_entry *entry = (struct acpi_srat_entry *)srattable;

        if (entry->type == 0)
        {

        }

        srattable += entry->length;

    }

}

void module_init()
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        struct smp_cpu *cpu = &architecture.cpus[i];

        cpu->id = 0;
        cpu->core = 0;
        cpu->chip = 0;
        cpu->domain = 0;

    }

    setmadt();
    setsrat();

    for (i = 0; i < architecture.count; i++)
    {

    }

}

