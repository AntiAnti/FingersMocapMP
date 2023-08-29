// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: mediapipe/modules/face_geometry/protos/environment.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3011000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3011004 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto;
namespace mediapipe {
namespace face_geometry {
class Environment;
class EnvironmentDefaultTypeInternal;
extern EnvironmentDefaultTypeInternal _Environment_default_instance_;
class PerspectiveCamera;
class PerspectiveCameraDefaultTypeInternal;
extern PerspectiveCameraDefaultTypeInternal _PerspectiveCamera_default_instance_;
}  // namespace face_geometry
}  // namespace mediapipe
PROTOBUF_NAMESPACE_OPEN
template<> ::mediapipe::face_geometry::Environment* Arena::CreateMaybeMessage<::mediapipe::face_geometry::Environment>(Arena*);
template<> ::mediapipe::face_geometry::PerspectiveCamera* Arena::CreateMaybeMessage<::mediapipe::face_geometry::PerspectiveCamera>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace mediapipe {
namespace face_geometry {

enum OriginPointLocation : int {
  BOTTOM_LEFT_CORNER = 1,
  TOP_LEFT_CORNER = 2
};
bool OriginPointLocation_IsValid(int value);
constexpr OriginPointLocation OriginPointLocation_MIN = BOTTOM_LEFT_CORNER;
constexpr OriginPointLocation OriginPointLocation_MAX = TOP_LEFT_CORNER;
constexpr int OriginPointLocation_ARRAYSIZE = OriginPointLocation_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* OriginPointLocation_descriptor();
template<typename T>
inline const std::string& OriginPointLocation_Name(T enum_t_value) {
  static_assert(::std::is_same<T, OriginPointLocation>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function OriginPointLocation_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    OriginPointLocation_descriptor(), enum_t_value);
}
inline bool OriginPointLocation_Parse(
    const std::string& name, OriginPointLocation* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<OriginPointLocation>(
    OriginPointLocation_descriptor(), name, value);
}
// ===================================================================

class PerspectiveCamera :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:mediapipe.face_geometry.PerspectiveCamera) */ {
 public:
  PerspectiveCamera();
  virtual ~PerspectiveCamera();

  PerspectiveCamera(const PerspectiveCamera& from);
  PerspectiveCamera(PerspectiveCamera&& from) noexcept
    : PerspectiveCamera() {
    *this = ::std::move(from);
  }

  inline PerspectiveCamera& operator=(const PerspectiveCamera& from) {
    CopyFrom(from);
    return *this;
  }
  inline PerspectiveCamera& operator=(PerspectiveCamera&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const PerspectiveCamera& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const PerspectiveCamera* internal_default_instance() {
    return reinterpret_cast<const PerspectiveCamera*>(
               &_PerspectiveCamera_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(PerspectiveCamera& a, PerspectiveCamera& b) {
    a.Swap(&b);
  }
  inline void Swap(PerspectiveCamera* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline PerspectiveCamera* New() const final {
    return CreateMaybeMessage<PerspectiveCamera>(nullptr);
  }

  PerspectiveCamera* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<PerspectiveCamera>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const PerspectiveCamera& from);
  void MergeFrom(const PerspectiveCamera& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(PerspectiveCamera* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "mediapipe.face_geometry.PerspectiveCamera";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto);
    return ::descriptor_table_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kVerticalFovDegreesFieldNumber = 1,
    kNearFieldNumber = 2,
    kFarFieldNumber = 3,
  };
  // optional float vertical_fov_degrees = 1;
  bool has_vertical_fov_degrees() const;
  private:
  bool _internal_has_vertical_fov_degrees() const;
  public:
  void clear_vertical_fov_degrees();
  float vertical_fov_degrees() const;
  void set_vertical_fov_degrees(float value);
  private:
  float _internal_vertical_fov_degrees() const;
  void _internal_set_vertical_fov_degrees(float value);
  public:

  // optional float near = 2;
  bool has_near() const;
  private:
  bool _internal_has_near() const;
  public:
  void clear_near();
  float near() const;
  void set_near(float value);
  private:
  float _internal_near() const;
  void _internal_set_near(float value);
  public:

  // optional float far = 3;
  bool has_far() const;
  private:
  bool _internal_has_far() const;
  public:
  void clear_far();
  float far() const;
  void set_far(float value);
  private:
  float _internal_far() const;
  void _internal_set_far(float value);
  public:

  // @@protoc_insertion_point(class_scope:mediapipe.face_geometry.PerspectiveCamera)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  float vertical_fov_degrees_;
  float near_;
  float far_;
  friend struct ::TableStruct_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto;
};
// -------------------------------------------------------------------

class Environment :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:mediapipe.face_geometry.Environment) */ {
 public:
  Environment();
  virtual ~Environment();

  Environment(const Environment& from);
  Environment(Environment&& from) noexcept
    : Environment() {
    *this = ::std::move(from);
  }

  inline Environment& operator=(const Environment& from) {
    CopyFrom(from);
    return *this;
  }
  inline Environment& operator=(Environment&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const Environment& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Environment* internal_default_instance() {
    return reinterpret_cast<const Environment*>(
               &_Environment_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(Environment& a, Environment& b) {
    a.Swap(&b);
  }
  inline void Swap(Environment* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline Environment* New() const final {
    return CreateMaybeMessage<Environment>(nullptr);
  }

  Environment* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<Environment>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const Environment& from);
  void MergeFrom(const Environment& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Environment* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "mediapipe.face_geometry.Environment";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto);
    return ::descriptor_table_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kPerspectiveCameraFieldNumber = 2,
    kOriginPointLocationFieldNumber = 1,
  };
  // optional .mediapipe.face_geometry.PerspectiveCamera perspective_camera = 2;
  bool has_perspective_camera() const;
  private:
  bool _internal_has_perspective_camera() const;
  public:
  void clear_perspective_camera();
  const ::mediapipe::face_geometry::PerspectiveCamera& perspective_camera() const;
  ::mediapipe::face_geometry::PerspectiveCamera* release_perspective_camera();
  ::mediapipe::face_geometry::PerspectiveCamera* mutable_perspective_camera();
  void set_allocated_perspective_camera(::mediapipe::face_geometry::PerspectiveCamera* perspective_camera);
  private:
  const ::mediapipe::face_geometry::PerspectiveCamera& _internal_perspective_camera() const;
  ::mediapipe::face_geometry::PerspectiveCamera* _internal_mutable_perspective_camera();
  public:

  // optional .mediapipe.face_geometry.OriginPointLocation origin_point_location = 1;
  bool has_origin_point_location() const;
  private:
  bool _internal_has_origin_point_location() const;
  public:
  void clear_origin_point_location();
  ::mediapipe::face_geometry::OriginPointLocation origin_point_location() const;
  void set_origin_point_location(::mediapipe::face_geometry::OriginPointLocation value);
  private:
  ::mediapipe::face_geometry::OriginPointLocation _internal_origin_point_location() const;
  void _internal_set_origin_point_location(::mediapipe::face_geometry::OriginPointLocation value);
  public:

  // @@protoc_insertion_point(class_scope:mediapipe.face_geometry.Environment)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  ::mediapipe::face_geometry::PerspectiveCamera* perspective_camera_;
  int origin_point_location_;
  friend struct ::TableStruct_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// PerspectiveCamera

// optional float vertical_fov_degrees = 1;
inline bool PerspectiveCamera::_internal_has_vertical_fov_degrees() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool PerspectiveCamera::has_vertical_fov_degrees() const {
  return _internal_has_vertical_fov_degrees();
}
inline void PerspectiveCamera::clear_vertical_fov_degrees() {
  vertical_fov_degrees_ = 0;
  _has_bits_[0] &= ~0x00000001u;
}
inline float PerspectiveCamera::_internal_vertical_fov_degrees() const {
  return vertical_fov_degrees_;
}
inline float PerspectiveCamera::vertical_fov_degrees() const {
  // @@protoc_insertion_point(field_get:mediapipe.face_geometry.PerspectiveCamera.vertical_fov_degrees)
  return _internal_vertical_fov_degrees();
}
inline void PerspectiveCamera::_internal_set_vertical_fov_degrees(float value) {
  _has_bits_[0] |= 0x00000001u;
  vertical_fov_degrees_ = value;
}
inline void PerspectiveCamera::set_vertical_fov_degrees(float value) {
  _internal_set_vertical_fov_degrees(value);
  // @@protoc_insertion_point(field_set:mediapipe.face_geometry.PerspectiveCamera.vertical_fov_degrees)
}

// optional float near = 2;
inline bool PerspectiveCamera::_internal_has_near() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool PerspectiveCamera::has_near() const {
  return _internal_has_near();
}
inline void PerspectiveCamera::clear_near() {
  near_ = 0;
  _has_bits_[0] &= ~0x00000002u;
}
inline float PerspectiveCamera::_internal_near() const {
  return near_;
}
inline float PerspectiveCamera::near() const {
  // @@protoc_insertion_point(field_get:mediapipe.face_geometry.PerspectiveCamera.near)
  return _internal_near();
}
inline void PerspectiveCamera::_internal_set_near(float value) {
  _has_bits_[0] |= 0x00000002u;
  near_ = value;
}
inline void PerspectiveCamera::set_near(float value) {
  _internal_set_near(value);
  // @@protoc_insertion_point(field_set:mediapipe.face_geometry.PerspectiveCamera.near)
}

// optional float far = 3;
inline bool PerspectiveCamera::_internal_has_far() const {
  bool value = (_has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool PerspectiveCamera::has_far() const {
  return _internal_has_far();
}
inline void PerspectiveCamera::clear_far() {
  far_ = 0;
  _has_bits_[0] &= ~0x00000004u;
}
inline float PerspectiveCamera::_internal_far() const {
  return far_;
}
inline float PerspectiveCamera::far() const {
  // @@protoc_insertion_point(field_get:mediapipe.face_geometry.PerspectiveCamera.far)
  return _internal_far();
}
inline void PerspectiveCamera::_internal_set_far(float value) {
  _has_bits_[0] |= 0x00000004u;
  far_ = value;
}
inline void PerspectiveCamera::set_far(float value) {
  _internal_set_far(value);
  // @@protoc_insertion_point(field_set:mediapipe.face_geometry.PerspectiveCamera.far)
}

// -------------------------------------------------------------------

// Environment

// optional .mediapipe.face_geometry.OriginPointLocation origin_point_location = 1;
inline bool Environment::_internal_has_origin_point_location() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool Environment::has_origin_point_location() const {
  return _internal_has_origin_point_location();
}
inline void Environment::clear_origin_point_location() {
  origin_point_location_ = 1;
  _has_bits_[0] &= ~0x00000002u;
}
inline ::mediapipe::face_geometry::OriginPointLocation Environment::_internal_origin_point_location() const {
  return static_cast< ::mediapipe::face_geometry::OriginPointLocation >(origin_point_location_);
}
inline ::mediapipe::face_geometry::OriginPointLocation Environment::origin_point_location() const {
  // @@protoc_insertion_point(field_get:mediapipe.face_geometry.Environment.origin_point_location)
  return _internal_origin_point_location();
}
inline void Environment::_internal_set_origin_point_location(::mediapipe::face_geometry::OriginPointLocation value) {
  assert(::mediapipe::face_geometry::OriginPointLocation_IsValid(value));
  _has_bits_[0] |= 0x00000002u;
  origin_point_location_ = value;
}
inline void Environment::set_origin_point_location(::mediapipe::face_geometry::OriginPointLocation value) {
  _internal_set_origin_point_location(value);
  // @@protoc_insertion_point(field_set:mediapipe.face_geometry.Environment.origin_point_location)
}

// optional .mediapipe.face_geometry.PerspectiveCamera perspective_camera = 2;
inline bool Environment::_internal_has_perspective_camera() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  PROTOBUF_ASSUME(!value || perspective_camera_ != nullptr);
  return value;
}
inline bool Environment::has_perspective_camera() const {
  return _internal_has_perspective_camera();
}
inline void Environment::clear_perspective_camera() {
  if (perspective_camera_ != nullptr) perspective_camera_->Clear();
  _has_bits_[0] &= ~0x00000001u;
}
inline const ::mediapipe::face_geometry::PerspectiveCamera& Environment::_internal_perspective_camera() const {
  const ::mediapipe::face_geometry::PerspectiveCamera* p = perspective_camera_;
  return p != nullptr ? *p : *reinterpret_cast<const ::mediapipe::face_geometry::PerspectiveCamera*>(
      &::mediapipe::face_geometry::_PerspectiveCamera_default_instance_);
}
inline const ::mediapipe::face_geometry::PerspectiveCamera& Environment::perspective_camera() const {
  // @@protoc_insertion_point(field_get:mediapipe.face_geometry.Environment.perspective_camera)
  return _internal_perspective_camera();
}
inline ::mediapipe::face_geometry::PerspectiveCamera* Environment::release_perspective_camera() {
  // @@protoc_insertion_point(field_release:mediapipe.face_geometry.Environment.perspective_camera)
  _has_bits_[0] &= ~0x00000001u;
  ::mediapipe::face_geometry::PerspectiveCamera* temp = perspective_camera_;
  perspective_camera_ = nullptr;
  return temp;
}
inline ::mediapipe::face_geometry::PerspectiveCamera* Environment::_internal_mutable_perspective_camera() {
  _has_bits_[0] |= 0x00000001u;
  if (perspective_camera_ == nullptr) {
    auto* p = CreateMaybeMessage<::mediapipe::face_geometry::PerspectiveCamera>(GetArenaNoVirtual());
    perspective_camera_ = p;
  }
  return perspective_camera_;
}
inline ::mediapipe::face_geometry::PerspectiveCamera* Environment::mutable_perspective_camera() {
  // @@protoc_insertion_point(field_mutable:mediapipe.face_geometry.Environment.perspective_camera)
  return _internal_mutable_perspective_camera();
}
inline void Environment::set_allocated_perspective_camera(::mediapipe::face_geometry::PerspectiveCamera* perspective_camera) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == nullptr) {
    delete perspective_camera_;
  }
  if (perspective_camera) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena = nullptr;
    if (message_arena != submessage_arena) {
      perspective_camera = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, perspective_camera, submessage_arena);
    }
    _has_bits_[0] |= 0x00000001u;
  } else {
    _has_bits_[0] &= ~0x00000001u;
  }
  perspective_camera_ = perspective_camera;
  // @@protoc_insertion_point(field_set_allocated:mediapipe.face_geometry.Environment.perspective_camera)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace face_geometry
}  // namespace mediapipe

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::mediapipe::face_geometry::OriginPointLocation> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::mediapipe::face_geometry::OriginPointLocation>() {
  return ::mediapipe::face_geometry::OriginPointLocation_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_mediapipe_2fmodules_2fface_5fgeometry_2fprotos_2fenvironment_2eproto