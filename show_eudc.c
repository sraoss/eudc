/*
 * Filename:  eudc_show.c
 * Description:  Function showing eudc installation
 * 		status.
 */

#include "postgres.h"
#include "funcapi.h"
#include "executor/spi.h"

#define sql "SELECT conproc::text,\n"\
	    "       pg_catalog.pg_encoding_to_char(conforencoding)::text,\n"\
	    "       pg_catalog.pg_encoding_to_char(contoencoding)::text,\n" \
	    "       CASE WHEN condefault::bool THEN 'yes' ELSE 'no' END \n" \
	    "FROM pg_conversion \n" \
	    "WHERE conproc::text like '%eudc%'"

PGDLLEXPORT PG_FUNCTION_INFO_V1(show_eudc);

Datum
show_eudc(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	HeapTupleHeader  *values = NULL;

	/*  Stuff done only on the first call of the function. */
	if (SRF_IS_FIRSTCALL()) {
		MemoryContext oldcontext, spicontext;

		int ret, i;
		TupleDesc tupdesc;
		SPITupleTable *tuptable = NULL;
		
		/*  create a function context for cross-call persistence */
		funcctx = SRF_FIRSTCALL_INIT();

		/*  switch to memory context appropriate for multiple function calls */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		SPI_connect();
		ret = SPI_exec(sql, 0);
		
		if (ret == SPI_OK_SELECT) {
			tupdesc = SPI_tuptable->tupdesc;
			tuptable = SPI_tuptable;
		} else {
			funcctx->max_calls = 0;
			elog(WARNING, "UNEXPECTED EXECUTION RESULT: %s %d\n%s",
								__FILE__, __LINE__, sql);
		}
		
		if(SPI_processed > 0){
			funcctx->max_calls = SPI_processed;
			/* Dont want allocate memory in SPI context */
			spicontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
			values = (HeapTupleHeader *)
					palloc(SPI_processed * sizeof(HeapTupleHeader));
			MemoryContextSwitchTo(spicontext);

			for(i=0; i < SPI_processed; i++)
				values[i] = SPI_returntuple(tuptable->vals[i], tupdesc);
		}
	
		/*  Build a tuple descriptor for our result type */
		if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE) {
			ereport(ERROR,
					(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					errmsg("function returning record called in context "
							"that cannot accept type record")));
		}

		/* Preserved result during sub-sequnce  call */
		funcctx->user_fctx = values;
		SPI_finish();
		MemoryContextSwitchTo(oldcontext);
	}/* End if(SRF_IS_FIRSTCALL) */

	/*  stuff done on every call of the function */
	funcctx = SRF_PERCALL_SETUP();

	if (funcctx->call_cntr < funcctx->max_calls) {
		values = (HeapTupleHeader *) funcctx->user_fctx;
		SRF_RETURN_NEXT(funcctx,
			PointerGetDatum(values[funcctx->call_cntr - 1]));
	} else {
		/*  Do when there is no more left. */
		SRF_RETURN_DONE(funcctx);
	} 
	
}/* end show_eudc */ 
