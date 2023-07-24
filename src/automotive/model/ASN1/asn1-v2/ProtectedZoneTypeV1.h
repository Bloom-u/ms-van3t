/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ITS-Container"
 * 	found in "ITS-Container.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example -R`
 */

#ifndef	_ProtectedZoneTypeV1_H_
#define	_ProtectedZoneTypeV1_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ProtectedZoneTypeV1 {
	ProtectedZoneTypeV1_cenDsrcTolling	= 0
	/*
	 * Enumeration is extensible
	 */
} e_ProtectedZoneTypeV1;

/* ProtectedZoneTypeV1 */
typedef long	 ProtectedZoneTypeV1_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_ProtectedZoneTypeV1_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_ProtectedZoneTypeV1;
extern const asn_INTEGER_specifics_t asn_SPC_ProtectedZoneTypeV1_specs_1;
asn_struct_free_f ProtectedZoneTypeV1_free;
asn_struct_print_f ProtectedZoneTypeV1_print;
asn_constr_check_f ProtectedZoneTypeV1_constraint;
ber_type_decoder_f ProtectedZoneTypeV1_decode_ber;
der_type_encoder_f ProtectedZoneTypeV1_encode_der;
xer_type_decoder_f ProtectedZoneTypeV1_decode_xer;
xer_type_encoder_f ProtectedZoneTypeV1_encode_xer;
oer_type_decoder_f ProtectedZoneTypeV1_decode_oer;
oer_type_encoder_f ProtectedZoneTypeV1_encode_oer;
per_type_decoder_f ProtectedZoneTypeV1_decode_uper;
per_type_encoder_f ProtectedZoneTypeV1_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _ProtectedZoneTypeV1_H_ */
#include "asn_internal.h"