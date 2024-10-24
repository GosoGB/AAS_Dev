// struct UA_MonitoredItem {
//     UA_TimerEntry delayedFreePointers;
//     LIST_ENTRY(UA_MonitoredItem) listEntry; /* Linked list in the Subscription */
//     UA_Subscription *subscription; /* If NULL, then this is a Local MonitoredItem */
//     UA_UInt32 monitoredItemId;

//     /* Status and Settings */
//     UA_ReadValueId itemToMonitor;
//     UA_MonitoringMode monitoringMode;
//     UA_TimestampsToReturn timestampsToReturn;
//     UA_Boolean registered;       /* Registered in the server / Subscription */
//     UA_DateTime triggeredUntil;  /* If the MonitoringMode is SAMPLING,
//                                   * triggering the MonitoredItem puts the latest
//                                   * Notification into the publishing queue (of
//                                   * the Subscription). In addition, the first
//                                   * new sample is also published (and not just
//                                   * sampled) if it occurs within the duration of
//                                   * one publishing cycle after the triggering. */

//     /* If the filter is a UA_DataChangeFilter: The DataChangeFilter always
//      * contains an absolute deadband definition. Part 8, §6.2 gives the
//      * following formula to test for percentage deadbands:
//      *
//      * DataChange if (absolute value of (last cached value - current value)
//      *                > (deadbandValue/100.0) * ((high–low) of EURange)))
//      *
//      * So we can convert from a percentage to an absolute deadband and keep
//      * the hot code path simple.
//      *
//      * TODO: Store the percentage deadband to recompute when the UARange is
//      * changed at runtime of the MonitoredItem */
//     UA_MonitoringParameters parameters;

//     /* Sampling */
//     UA_MonitoredItemSamplingType samplingType;
//     union {
//         UA_UInt64 callbackId;
//         UA_MonitoredItem *nodeListNext; /* Event-Based: Attached to Node */
//         LIST_ENTRY(UA_MonitoredItem) samplingListEntry; /* Publish-interval: Linked in
//                                                          * Subscription */
//     } sampling;
//     UA_DataValue lastValue;

//     /* Triggering Links */
//     size_t triggeringLinksSize;
//     UA_UInt32 *triggeringLinks;

//     /* Notification Queue */
//     NotificationQueue queue;
//     size_t queueSize; /* This is the current size. See also the configured
//                        * (maximum) queueSize in the parameters. */
//     size_t eventOverflows; /* Separate counter for the queue. Can at most double
//                             * the queue size */
// };