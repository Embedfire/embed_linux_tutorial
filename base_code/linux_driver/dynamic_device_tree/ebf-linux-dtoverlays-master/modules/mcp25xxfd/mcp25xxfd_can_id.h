/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CAN_IF_H
#define __MCP25XXFD_CAN_IF_H

#include <uapi/linux/can.h>

#include "mcp25xxfd_can_id.h"
#include "mcp25xxfd_regs.h"

/* ideally these would be defined in uapi/linux/can.h */
#define MCP25XXFD_CAN_EFF_SID_SHIFT	(CAN_EFF_ID_BITS - CAN_SFF_ID_BITS)
#define MCP25XXFD_CAN_EFF_SID_BITS	CAN_SFF_ID_BITS
#define MCP25XXFD_CAN_EFF_SID_MASK					\
	GENMASK(MCP25XXFD_CAN_EFF_SID_SHIFT + MCP25XXFD_CAN_EFF_SID_BITS - 1, \
		MCP25XXFD_CAN_EFF_SID_SHIFT)
#define MCP25XXFD_CAN_EFF_EID_SHIFT	0
#define MCP25XXFD_CAN_EFF_EID_BITS	MCP25XXFD_CAN_EFF_SID_SHIFT
#define MCP25XXFD_CAN_EFF_EID_MASK					\
	GENMASK(MCP25XXFD_CAN_EFF_EID_SHIFT + MCP25XXFD_CAN_EFF_EID_BITS - 1, \
		MCP25XXFD_CAN_EFF_EID_SHIFT)

static inline
void mcp25xxfd_can_id_from_mcp25xxfd(u32 mcp_id, u32 mcp_flags, u32 *can_id)
{
	u32 sid = (mcp_id & MCP25XXFD_CAN_OBJ_ID_SID_MASK) >>
		MCP25XXFD_CAN_OBJ_ID_SID_SHIFT;
	u32 eid = (mcp_id & MCP25XXFD_CAN_OBJ_ID_EID_MASK) >>
		MCP25XXFD_CAN_OBJ_ID_EID_SHIFT;

	/* select normal or extended ids */
	if (mcp_flags & MCP25XXFD_CAN_OBJ_FLAGS_IDE) {
		*can_id = (eid << MCP25XXFD_CAN_EFF_EID_SHIFT) |
			(sid << MCP25XXFD_CAN_EFF_SID_SHIFT) |
			CAN_EFF_FLAG;
	} else {
		*can_id = sid << MCP25XXFD_CAN_EFF_EID_SHIFT;
	}
	/* handle rtr */
	*can_id |= (mcp_flags & MCP25XXFD_CAN_OBJ_FLAGS_RTR) ? CAN_RTR_FLAG : 0;
}

static inline
void mcp25xxfd_can_id_to_mcp25xxfd(u32 can_id, u32 *id, u32 *flags)
{
	/* depending on can_id flag compute extended or standard ids */
	if (can_id & CAN_EFF_FLAG) {
		int sid = (can_id & MCP25XXFD_CAN_EFF_SID_MASK) >>
			MCP25XXFD_CAN_EFF_SID_SHIFT;
		int eid = (can_id & MCP25XXFD_CAN_EFF_EID_MASK) >>
			MCP25XXFD_CAN_EFF_EID_SHIFT;
		*id = (eid << MCP25XXFD_CAN_OBJ_ID_EID_SHIFT) |
			(sid << MCP25XXFD_CAN_OBJ_ID_SID_SHIFT);
		*flags = MCP25XXFD_CAN_OBJ_FLAGS_IDE;
	} else {
		*id = can_id & CAN_SFF_MASK;
		*flags = 0;
	}

	/* Handle RTR */
	*flags |= (can_id & CAN_RTR_FLAG) ? MCP25XXFD_CAN_OBJ_FLAGS_RTR : 0;
}

#endif /* __MCP25XXFD_CAN_IF_H */
