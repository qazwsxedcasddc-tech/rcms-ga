#pragma once

#include <QString>
#include <cstdint>

namespace rcms {

/**
 * @brief Device metadata for identification and display
 *
 * Contains both hardware-read and user-editable fields
 */
struct DeviceMetadata {
    // Primary key (used for device matching)
    QString profileId;              // Connection profile ID
    uint8_t modbusAddress = 1;      // Device address on bus (1-247)

    // Hardware-read fields (read-only)
    QString hardwareId;             // Serial number / hardware ID from device
    QString model;                  // Device model (e.g., "Fazan-19 P50")
    QString firmwareVersion;        // Firmware version

    // User-editable fields
    QString alias;                  // User-friendly name (e.g., "Tower-1 Main")
    QString inventoryNumber;        // Inventory / asset number
    QString location;               // Physical location description
    QString comment;                // Free-form comment
    QString groupId;                // Group this device belongs to

    /**
     * @brief Get display name (alias if set, otherwise generated)
     */
    QString displayName() const {
        if (!alias.isEmpty()) {
            return alias;
        }
        return QString("Device #%1").arg(modbusAddress);
    }

    /**
     * @brief Get composite identifier string
     */
    QString compositeId() const {
        QString result = QString("Addr:%1").arg(modbusAddress);
        if (!hardwareId.isEmpty()) {
            result += QString(" | HW:%1").arg(hardwareId);
        }
        if (!inventoryNumber.isEmpty()) {
            result += QString(" | Inv:%1").arg(inventoryNumber);
        }
        return result;
    }
};

} // namespace rcms
