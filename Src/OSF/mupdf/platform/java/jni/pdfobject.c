/* PDFObject interface */

JNIEXPORT void JNICALL FUN(PDFObject_finalize)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject_safe(env, self);
	if(!ctx || !obj) return;
	env->SetLongField(self, fid_PDFObject_pointer, 0);
	pdf_drop_obj(ctx, obj);
}

JNIEXPORT jint JNICALL FUN(PDFObject_toIndirect)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int num = 0;

	if(!ctx || !obj) return 0;

	fz_try(ctx)
	num = pdf_to_num(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return num;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isIndirect)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_indirect(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isNull)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_null(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isBoolean)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_bool(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isInteger)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_int(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isReal)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_real(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isNumber)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_number(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isString)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_string(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isName)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_name(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isArray)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_array(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isDictionary)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_dict(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_isStream)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_is_stream(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jbyteArray JNICALL FUN(PDFObject_readStream)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	fz_buffer * buf = NULL;
	jbyteArray arr = NULL;

	if(!ctx || !obj) return NULL;

	fz_var(buf);
	fz_try(ctx)
	{
		unsigned char * data;
		size_t len;

		buf = pdf_load_stream(ctx, obj);
		len = fz_buffer_storage(ctx, buf, &data);

		arr = env->NewByteArray((jsize)len);
		if(env->ExceptionCheck())
			fz_throw_java(ctx, env);
		if(!arr)
			fz_throw(ctx, FZ_ERROR_GENERIC, "can not create byte array");

		env->SetByteArrayRegion(arr, 0, (jsize)len, (signed char*)data);
		if(env->ExceptionCheck())
			fz_throw_java(ctx, env);
	}
	fz_always(ctx)
	fz_drop_buffer(ctx, buf);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return arr;
}

JNIEXPORT jbyteArray JNICALL FUN(PDFObject_readRawStream)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	fz_buffer * buf = NULL;
	jbyteArray arr = NULL;

	if(!ctx || !obj) return NULL;

	fz_var(buf);
	fz_try(ctx)
	{
		unsigned char * data;
		size_t len;

		buf = pdf_load_raw_stream(ctx, obj);
		len = fz_buffer_storage(ctx, buf, &data);

		arr = env->NewByteArray((jsize)len);
		if(env->ExceptionCheck())
			fz_throw_java(ctx, env);
		if(!arr)
			fz_throw(ctx, FZ_ERROR_GENERIC, "cannot create byte array");

		env->SetByteArrayRegion(arr, 0, (jsize)len, (signed char*)&data[0]);
		if(env->ExceptionCheck())
			fz_throw_java(ctx, env);
	}
	fz_always(ctx)
	fz_drop_buffer(ctx, buf);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return arr;
}

JNIEXPORT void JNICALL FUN(PDFObject_writeObject)(JNIEnv *env, jobject self, jobject jobj)
{
	fz_context * ctx = get_context(env);
	pdf_obj * ref = from_PDFObject(env, self);
	pdf_document * pdf = pdf_get_bound_document(ctx, ref);
	pdf_obj * obj = from_PDFObject(env, jobj);

	if(!ctx || !obj) return;
	if(!pdf) jni_throw_arg_void(env, "object not bound to document");
	if(!obj) jni_throw_arg_void(env, "object must not be null");

	fz_try(ctx)
	pdf_update_object(ctx, pdf, pdf_to_num(ctx, ref), obj);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_writeStreamBuffer)(JNIEnv *env, jobject self, jobject jbuf)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	pdf_document * pdf = pdf_get_bound_document(ctx, obj);
	fz_buffer * buf = from_Buffer(env, jbuf);

	if(!ctx || !obj) return;
	if(!pdf) jni_throw_arg_void(env, "object not bound to document");
	if(!buf) jni_throw_arg_void(env, "buffer must not be null");

	fz_try(ctx)
	pdf_update_stream(ctx, pdf, obj, buf, 0);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_writeStreamString)(JNIEnv *env, jobject self, jstring jstr)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	pdf_document * pdf = pdf_get_bound_document(ctx, obj);
	const char * str = NULL;
	fz_buffer * buf = NULL;

	if(!ctx || !obj) return;
	if(!pdf) jni_throw_arg_void(env, "object not bound to document");
	if(!jstr) jni_throw_arg_void(env, "string must not be null");

	str = env->GetStringUTFChars(jstr, NULL);
	if(!str) return;

	fz_var(buf);

	fz_try(ctx)
	{
		buf = fz_new_buffer_from_copied_data(ctx, (const unsigned char*)str, strlen(str));
		pdf_update_stream(ctx, pdf, obj, buf, 0);
	}
	fz_always(ctx)
	{
		fz_drop_buffer(ctx, buf);
		env->ReleaseStringUTFChars(jstr, str);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_writeRawStreamBuffer)(JNIEnv *env, jobject self, jobject jbuf)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	pdf_document * pdf = pdf_get_bound_document(ctx, obj);
	fz_buffer * buf = from_Buffer(env, jbuf);

	if(!ctx || !obj) return;
	if(!pdf) jni_throw_arg_void(env, "object not bound to document");
	if(!buf) jni_throw_arg_void(env, "buffer must not be null");

	fz_try(ctx)
	pdf_update_stream(ctx, pdf, obj, buf, 1);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_writeRawStreamString)(JNIEnv *env, jobject self, jstring jstr)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	pdf_document * pdf = pdf_get_bound_document(ctx, obj);
	const char * str = NULL;
	fz_buffer * buf = NULL;

	if(!ctx || !obj) return;
	if(!pdf) jni_throw_arg_void(env, "object not bound to document");
	if(!jstr) jni_throw_arg_void(env, "string must not be null");

	str = env->GetStringUTFChars(jstr, NULL);
	if(!str) return;

	fz_var(buf);

	fz_try(ctx)
	{
		buf = fz_new_buffer_from_copied_data(ctx, (const unsigned char*)str, strlen(str));
		pdf_update_stream(ctx, pdf, obj, buf, 1);
	}
	fz_always(ctx)
	{
		fz_drop_buffer(ctx, buf);
		env->ReleaseStringUTFChars(jstr, str);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT jobject JNICALL FUN(PDFObject_resolve)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	pdf_obj * ind = NULL;
	jobject jobj;

	if(!ctx || !obj) return NULL;

	fz_try(ctx)
	ind = pdf_resolve_indirect(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	pdf_keep_obj(ctx, ind);
	jobj = env->NewObject(cls_PDFObject, mid_PDFObject_init, jlong_cast(ind));
	if(!jobj)
		pdf_drop_obj(ctx, ind);
	return jobj;
}

JNIEXPORT jobject JNICALL FUN(PDFObject_getArray)(JNIEnv *env, jobject self, jint index)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);
	pdf_obj * val = NULL;

	if(!ctx || !arr) return NULL;

	fz_try(ctx)
	val = pdf_array_get(ctx, arr, index);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return to_PDFObject_safe(ctx, env, val);
}

JNIEXPORT jobject JNICALL FUN(PDFObject_getDictionary)(JNIEnv *env, jobject self, jstring jname)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	const char * name = NULL;
	pdf_obj * val = NULL;

	if(!ctx || !dict) return NULL;
	if(!jname) jni_throw_arg(env, "name must not be null");

	name = env->GetStringUTFChars(jname, NULL);
	if(!name) jni_throw_run(env, "cannot get name to lookup");

	fz_try(ctx)
	val = pdf_dict_gets(ctx, dict, name);
	fz_always(ctx)
		env->ReleaseStringUTFChars(jname, name);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return to_PDFObject_safe(ctx, env, val);
}

JNIEXPORT jobject JNICALL FUN(PDFObject_getDictionaryKey)(JNIEnv *env, jobject self, jint index)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * key = NULL;

	if(!ctx || !dict) return NULL;

	fz_try(ctx)
	key = pdf_dict_get_key(ctx, dict, index);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return to_PDFObject_safe(ctx, env, key);
}

JNIEXPORT void JNICALL FUN(PDFObject_putArrayBoolean)(JNIEnv *env, jobject self, jint index, jboolean b)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_put(ctx, arr, index, b ? PDF_TRUE : PDF_FALSE);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putArrayInteger)(JNIEnv *env, jobject self, jint index, jint i)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_put_drop(ctx, arr, index, pdf_new_int(ctx, i));
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putArrayFloat)(JNIEnv *env, jobject self, jint index, jfloat f)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_put_drop(ctx, arr, index, pdf_new_real(ctx, f));
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putArrayString)(JNIEnv *env, jobject self, jint index, jstring jstr)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);
	const char * str = NULL;

	if(!ctx || !arr) return;
	if(jstr) {
		str = env->GetStringUTFChars(jstr, NULL);
		if(!str) return;
	}

	fz_try(ctx)
	{
		if(str)
			pdf_array_put_drop(ctx, arr, index, pdf_new_string(ctx, str, strlen(str)));
		else
			pdf_array_put(ctx, arr, index, PDF_NULL);
	}
	fz_always(ctx)
	{
		if(str)
			env->ReleaseStringUTFChars(jstr, str);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putArrayPDFObject)(JNIEnv *env, jobject self, jint index, jobject jobj)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);
	pdf_obj * obj = from_PDFObject(env, jobj);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_put(ctx, arr, index, obj);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryStringBoolean)(JNIEnv *env, jobject self, jstring jname, jboolean b)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	const char * name = NULL;
	pdf_obj * key = NULL;

	if(!ctx || !dict) return;
	if(jname) {
		name = env->GetStringUTFChars(jname, NULL);
		if(!name) return;
	}

	fz_var(key);

	fz_try(ctx)
	{
		key = name ? pdf_new_name(ctx, name) : NULL;
		pdf_dict_put(ctx, dict, key, b ? PDF_TRUE : PDF_FALSE);
	}
	fz_always(ctx)
	{
		pdf_drop_obj(ctx, key);
		if(name)
			env->ReleaseStringUTFChars(jname, name);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryStringInteger)(JNIEnv *env, jobject self, jstring jname, jint i)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	const char * name = NULL;
	pdf_obj * key = NULL;

	if(!ctx || !dict) return;
	if(jname) {
		name = env->GetStringUTFChars(jname, NULL);
		if(!name) return;
	}

	fz_var(key);

	fz_try(ctx)
	{
		key = name ? pdf_new_name(ctx, name) : NULL;
		pdf_dict_put_int(ctx, dict, key, i);
	}
	fz_always(ctx)
	{
		pdf_drop_obj(ctx, key);
		if(name)
			env->ReleaseStringUTFChars(jname, name);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryStringFloat)(JNIEnv *env, jobject self, jstring jname, jfloat f)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	const char * name = NULL;
	pdf_obj * key = NULL;

	if(!ctx || !dict) return;
	if(jname) {
		name = env->GetStringUTFChars(jname, NULL);
		if(!name) return;
	}

	fz_var(key);

	fz_try(ctx)
	{
		key = name ? pdf_new_name(ctx, name) : NULL;
		pdf_dict_put_real(ctx, dict, key, f);
	}
	fz_always(ctx)
	{
		pdf_drop_obj(ctx, key);
		if(name)
			env->ReleaseStringUTFChars(jname, name);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryStringString)(JNIEnv *env, jobject self, jstring jname, jstring jstr)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	const char * name = NULL;
	const char * str = NULL;
	pdf_obj * key = NULL;

	if(!ctx || !dict) return;
	if(jname) {
		name = env->GetStringUTFChars(jname, NULL);
		if(!name) return;
	}
	if(jstr) {
		str = env->GetStringUTFChars(jstr, NULL);
		if(!str) {
			env->ReleaseStringUTFChars(jname, str);
			return;
		}
	}

	fz_var(key);

	fz_try(ctx)
	{
		key = name ? pdf_new_name(ctx, name) : NULL;
		if(str)
			pdf_dict_put_string(ctx, dict, key, str, strlen(str));
		else
			pdf_dict_put(ctx, dict, key, PDF_NULL);
	}
	fz_always(ctx)
	{
		pdf_drop_obj(ctx, key);
		if(str)
			env->ReleaseStringUTFChars(jstr, str);
		if(name)
			env->ReleaseStringUTFChars(jname, name);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryStringPDFObject)(JNIEnv *env, jobject self, jstring jname, jobject jobj)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * val = from_PDFObject(env, jobj);
	const char * name = NULL;
	pdf_obj * key = NULL;

	if(!ctx || !dict) return;
	if(jname) {
		name = env->GetStringUTFChars(jname, NULL);
		if(!name) return;
	}

	fz_var(key);

	fz_try(ctx)
	{
		key = name ? pdf_new_name(ctx, name) : NULL;
		pdf_dict_put(ctx, dict, key, val);
	}
	fz_always(ctx)
	{
		pdf_drop_obj(ctx, key);
		if(name)
			env->ReleaseStringUTFChars(jname, name);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectBoolean)(JNIEnv *env, jobject self, jobject jname, jboolean b)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_put(ctx, dict, name, b ? PDF_TRUE : PDF_FALSE);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectInteger)(JNIEnv *env, jobject self, jobject jname, jint i)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_put_int(ctx, dict, name, i);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectFloat)(JNIEnv *env, jobject self, jobject jname, jfloat f)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_put_real(ctx, dict, name, f);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectString)(JNIEnv *env, jobject self, jobject jname, jstring jstr)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);
	const char * str = NULL;

	if(!ctx || !dict) return;
	if(jstr) {
		str = env->GetStringUTFChars(jstr, NULL);
		if(!str) return;
	}

	fz_try(ctx)
	{
		if(str)
			pdf_dict_put_string(ctx, dict, name, str, strlen(str));
		else
			pdf_dict_put(ctx, dict, name, PDF_NULL);
	}
	fz_always(ctx)
	{
		if(str)
			env->ReleaseStringUTFChars(jstr, str);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectPDFObject)(JNIEnv *env, jobject self, jobject jname, jobject jobj)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);
	pdf_obj * obj = from_PDFObject(env, jobj);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_put(ctx, dict, name, obj);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectRect)(JNIEnv *env, jobject self, jobject jname, jobject jrect)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);
	fz_rect rect  = from_Rect(env, jrect);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_put_rect(ctx, dict, name, rect);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectMatrix)(JNIEnv *env, jobject self, jobject jname, jobject jmatrix)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);
	fz_matrix matrix = from_Matrix(env, jmatrix);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_put_matrix(ctx, dict, name, matrix);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_putDictionaryPDFObjectDate)(JNIEnv *env, jobject self, jobject jname, jlong secs)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_put_date(ctx, dict, name, secs);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_deleteArray)(JNIEnv *env, jobject self, jint index)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_delete(ctx, arr, index);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_deleteDictionaryString)(JNIEnv *env, jobject self, jstring jname)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	const char * name = NULL;

	if(!ctx || !dict) return;
	if(jname) {
		name = env->GetStringUTFChars(jname, NULL);
		if(!name) return;
	}

	fz_try(ctx)
	{
		pdf_dict_dels(ctx, dict, name);
	}
	fz_always(ctx)
	{
		if(name)
			env->ReleaseStringUTFChars(jname, name);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_deleteDictionaryPDFObject)(JNIEnv *env, jobject self, jobject jname)
{
	fz_context * ctx = get_context(env);
	pdf_obj * dict = from_PDFObject(env, self);
	pdf_obj * name = from_PDFObject(env, jname);

	if(!ctx || !dict) return;

	fz_try(ctx)
	pdf_dict_del(ctx, dict, name);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT jboolean JNICALL FUN(PDFObject_asBoolean)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int b = 0;

	if(!ctx || !obj) return JNI_FALSE;

	fz_try(ctx)
	b = pdf_to_bool(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return b ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL FUN(PDFObject_asInteger)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int i = 0;

	if(!ctx || !obj) return 0;

	fz_try(ctx)
	i = pdf_to_int(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return i;
}

JNIEXPORT jfloat JNICALL FUN(PDFObject_asFloat)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	float f = 0;

	if(!ctx || !obj) return 0;

	fz_try(ctx)
	f = pdf_to_real(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return f;
}

JNIEXPORT jint JNICALL FUN(PDFObject_asIndirect)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int ind = 0;

	if(!ctx || !obj) return 0;

	fz_try(ctx)
	ind = pdf_to_num(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return ind;
}

JNIEXPORT jstring JNICALL FUN(PDFObject_asString)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	const char * str = NULL;

	if(!ctx || !obj) return NULL;

	fz_try(ctx)
	str = pdf_to_text_string(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return env->NewStringUTF(str);
}

JNIEXPORT /*jobject*/jbyteArray JNICALL FUN(PDFObject_asByteString)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	const char * str = NULL;
	/*jobject*/jbyteArray jbs = NULL;
	jbyte * bs = NULL;
	size_t len;
	if(!ctx || !obj) return NULL;
	fz_try(ctx)
	{
		str = pdf_to_str_buf(ctx, obj);
		len = pdf_to_str_len(ctx, obj);
	}
	fz_catch(ctx)
	jni_rethrow(env, ctx);
	jbs = env->NewByteArray(len);
	if(env->ExceptionCheck()) return NULL;
	if(!jbs) jni_throw_run(env, "cannot create byte array");
	bs = env->GetByteArrayElements((jbyteArray)jbs, NULL);
	if(!bs) return NULL;
	memcpy(bs, str, len);
	env->ReleaseByteArrayElements((jbyteArray)jbs, bs, 0);
	return jbs;
}

JNIEXPORT jstring JNICALL FUN(PDFObject_asName)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	const char * str = NULL;
	if(!ctx || !obj) return NULL;
	fz_try(ctx)
	str = pdf_to_name(ctx, obj);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return env->NewStringUTF(str);
}

JNIEXPORT jint JNICALL FUN(PDFObject_size)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject(env, self);
	int len;

	if(!ctx || !obj) return 0;

	fz_try(ctx)
	{
		if(pdf_is_array(ctx, obj))
			len = pdf_array_len(ctx, obj);
		else if(pdf_is_dict(ctx, obj))
			len = pdf_dict_len(ctx, obj);
		else
			len = 0;
	}
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return len;
}

JNIEXPORT void JNICALL FUN(PDFObject_pushBoolean)(JNIEnv *env, jobject self, jboolean b)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);
	if(!ctx || !arr) return;
	fz_try(ctx)
	pdf_array_push_bool(ctx, arr, b);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_pushInteger)(JNIEnv *env, jobject self, jint i)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_push_int(ctx, arr, i);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_pushFloat)(JNIEnv *env, jobject self, jfloat f)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_push_real(ctx, arr, f);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_pushString)(JNIEnv *env, jobject self, jstring jstr)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);
	const char * str = NULL;

	if(!ctx || !arr) return;
	if(jstr) {
		str = env->GetStringUTFChars(jstr, NULL);
		if(!str) return;
	}

	fz_try(ctx)
	{
		if(str)
			pdf_array_push_string(ctx, arr, str, strlen(str));
		else
			pdf_array_push(ctx, arr, PDF_NULL);
	}
	fz_always(ctx)
	{
		if(str)
			env->ReleaseStringUTFChars(jstr, str);
	}
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(PDFObject_pushPDFObject)(JNIEnv *env, jobject self, jobject jitem)
{
	fz_context * ctx = get_context(env);
	pdf_obj * arr = from_PDFObject(env, self);
	pdf_obj * item = from_PDFObject(env, jitem);

	if(!ctx || !arr) return;

	fz_try(ctx)
	pdf_array_push(ctx, arr, item);
	fz_always(ctx)
	pdf_drop_obj(ctx, item);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT jstring JNICALL FUN(PDFObject_toString)(JNIEnv *env, jobject self, jboolean tight, jboolean ascii)
{
	fz_context * ctx = get_context(env);
	pdf_obj * obj = from_PDFObject_safe(env, self);
	jstring string = NULL;
	char * s = NULL;
	size_t n = 0;

	if(!ctx || !obj) return NULL;

	fz_var(s);

	fz_try(ctx)
	{
		s = pdf_sprint_obj(ctx, NULL, 0, &n, obj, tight, ascii);
		string = env->NewStringUTF(s);
	}
	fz_always(ctx)
	fz_free(ctx, s);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return string;
}
