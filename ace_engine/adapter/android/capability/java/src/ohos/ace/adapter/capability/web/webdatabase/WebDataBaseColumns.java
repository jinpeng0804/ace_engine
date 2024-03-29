/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ohos.ace.adapter.capability.web;

import android.provider.BaseColumns;

public class WebDataBaseColumns {
    private WebDataBaseColumns() {}

    public static class EntryColumns implements BaseColumns {
        public static final String TABLE_NAME_CREDENTIAL = "credential";
        public static final String COLUMN_NAME_USERNAME = "username";
        public static final String COLUMN_NAME_USERPASS = "password";
        public static final String COLUMN_NAME_HTTP_AUTH_ID = "http_auth_id";
        public static final String TABLE_NAME_HTTPAUTH = "http_auth";
        public static final String COLUMN_NAME_HOST = "host";
        public static final String COLUMN_NAME_REALM = "realm";
    }
}
