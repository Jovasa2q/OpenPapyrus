/* Text interface */

JNIEXPORT void JNICALL FUN(Text_finalize)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	fz_text * text = from_Text_safe(env, self);
	if(!ctx || !text) return;
	env->SetLongField(self, fid_Text_pointer, 0);
	fz_drop_text(ctx, text);
}

JNIEXPORT jlong JNICALL FUN(Text_newNative)(JNIEnv *env, jobject self)
{
	fz_context * ctx = get_context(env);
	fz_text * text = NULL;

	if(!ctx) return 0;

	fz_try(ctx)
	text = fz_new_text(ctx);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return jlong_cast(text);
}

JNIEXPORT jobject JNICALL FUN(Text_getBounds)(JNIEnv *env, jobject self, jobject jstroke, jobject jctm)
{
	fz_context * ctx = get_context(env);
	fz_text * text = from_Text(env, self);
	fz_stroke_state * stroke = from_StrokeState(env, jstroke);
	fz_matrix ctm = from_Matrix(env, jctm);
	fz_rect rect;

	if(!ctx || !text) return NULL;
	if(!stroke) jni_throw_arg(env, "stroke must not be null");

	fz_try(ctx)
	rect = fz_bound_text(ctx, text, stroke, ctm);
	fz_catch(ctx)
	jni_rethrow(env, ctx);

	return to_Rect_safe(ctx, env, rect);
}

JNIEXPORT void JNICALL FUN(Text_showGlyph)(JNIEnv *env, jobject self, jobject jfont, jobject jtrm, jint glyph, jint unicode, jboolean wmode)
{
	fz_context * ctx = get_context(env);
	fz_text * text = from_Text(env, self);
	fz_font * font = from_Font(env, jfont);
	fz_matrix trm = from_Matrix(env, jtrm);

	if(!ctx || !text) return;
	if(!font) jni_throw_arg_void(env, "font must not be null");

	fz_try(ctx)
	fz_show_glyph(ctx, text, font, trm, glyph, unicode, wmode, 0, FZ_BIDI_NEUTRAL, FZ_LANG_UNSET);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
}

JNIEXPORT void JNICALL FUN(Text_showString)(JNIEnv *env, jobject self, jobject jfont, jobject jtrm, jstring jstr, jboolean wmode)
{
	fz_context * ctx = get_context(env);
	fz_text * text = from_Text(env, self);
	fz_font * font = from_Font(env, jfont);
	fz_matrix trm = from_Matrix(env, jtrm);
	const char * str = NULL;

	if(!ctx || !text) return;
	if(!jfont) jni_throw_arg_void(env, "font must not be null");
	if(!jstr) jni_throw_arg_void(env, "string must not be null");
	str = env->GetStringUTFChars(jstr, NULL);
	if(!str) return;
	fz_try(ctx)
	trm = fz_show_string(ctx, text, font, trm, str, wmode, 0, FZ_BIDI_NEUTRAL, FZ_LANG_UNSET);
	fz_always(ctx)
		env->ReleaseStringUTFChars(jstr, str);
	fz_catch(ctx)
	jni_rethrow_void(env, ctx);
	env->SetFloatField(jtrm, fid_Matrix_e, trm.e);
	env->SetFloatField(jtrm, fid_Matrix_f, trm.f);
}

JNIEXPORT void JNICALL FUN(Text_walk)(JNIEnv *env, jobject self, jobject walker)
{
	fz_context * ctx = get_context(env);
	fz_text * text = from_Text(env, self);
	fz_text_span * span;
	fz_font * font = NULL;
	jobject jfont = NULL;
	jobject jtrm = NULL;
	int i;

	if(!ctx || !text) return;
	if(!walker) jni_throw_arg_void(env, "walker must not be null");

	if(text->head == NULL)
		return; /* text has no spans to walk */

	for(span = text->head; span; span = span->next) {
		if(font != span->font) {
			if(jfont)
				env->DeleteLocalRef(jfont);
			font = span->font;
			jfont = to_Font_safe(ctx, env, font);
			if(!jfont)
				return;
		}

		for(i = 0; i < span->len; ++i) {
			jtrm = env->NewObject(cls_Matrix, mid_Matrix_init,
				span->trm.a, span->trm.b, span->trm.c, span->trm.d,
				span->items[i].x, span->items[i].y);
			if(!jtrm) return;

			env->CallVoidMethod(walker, mid_TextWalker_showGlyph,
			    jfont, jtrm,
			    (jint)span->items[i].gid,
			    (jint)span->items[i].ucs,
			    (jint)span->wmode);
			if(env->ExceptionCheck()) return;

			env->DeleteLocalRef(jtrm);
		}
	}
}
